import sys
import time
import json
import gc
from pathlib import Path
from concurrent.futures import ThreadPoolExecutor
from hashlib import md5
from ollama import Client
from sentence_transformers import SentenceTransformer
import faiss
import numpy as np

class OptimizedCppRAG:
    def __init__(self):
        self.ollama = Client(host='http://localhost:11434')
        self.emb_model = None
        self.index = None
        self.metadata = []
        self.ignore_dirs = {'build', 'cmake-build', 'thirdparty', 'test', 'docs'}
        self.chunk_size = 256
        self.overlap = 32
        self.base_dir = Path(r'D:\AI\RagLoangGas')
        self.index_path = self.base_dir / 'vector_store'
        self.cache_path = self.base_dir / 'file_hashes.json'
        self._init_embedding_model()

    def _init_embedding_model(self):
        if self.emb_model is not None:
            del self.emb_model
            gc.collect()
        self.emb_model = SentenceTransformer('all-MiniLM-L6-v2', device='cpu')

    def _collect_files(self, path: Path):
        extensions = {'cpp', 'hpp', 'h', 'cc', 'cxx', 'c', 'inl'}
        for file_path in path.rglob('*'):
            if file_path.suffix[1:] not in extensions:
                continue
            if any(part in self.ignore_dirs for part in file_path.parts):
                continue
            if file_path.is_file() and file_path.stat().st_size < 2 * 1024 * 1024:
                yield file_path

    # Добавленные методы
    def _load_existing_index(self):
        """Загрузка существующего индекса"""
        if (self.index_path / 'index.faiss').exists():
            try:
                self.index = faiss.read_index(str(self.index_path / 'index.faiss'))
                with open(self.index_path / 'metadata.json', 'r') as f:
                    self.metadata = json.load(f)
                return True
            except Exception as e:
                print(f"Ошибка загрузки индекса: {str(e)}")
        return False

    def _save_index(self):
        """Сохранение индекса"""
        self.index_path.mkdir(exist_ok=True)
        faiss.write_index(self.index, str(self.index_path / 'index.faiss'))
        with open(self.index_path / 'metadata.json', 'w') as f:
            json.dump(self.metadata, f)

    def _get_file_hashes(self, files):
        return {str(f): md5(f.read_bytes()).hexdigest() for f in files}

    def _save_file_hashes(self, hashes):
        with open(self.cache_path, 'w') as f:
            json.dump(hashes, f)

    def _get_changed_files(self, current_hashes):
        try:
            with open(self.cache_path, 'r') as f:
                old_hashes = json.load(f)
            return [Path(f) for f, h in current_hashes.items() if old_hashes.get(f) != h]
        except FileNotFoundError:
            return [Path(f) for f in current_hashes.keys()]

    def _print_progress(self, processed: int, total: int):
        progress = processed / total
        bar = '#' * int(progress * 20)
        sys.stdout.write(f"\r[{bar.ljust(20)}] {processed}/{total} ({progress:.1%})")
        sys.stdout.flush()

    def index_codebase(self):
        if self._load_existing_index():
            print("Загружен существующий индекс")
            return

        source_dir = self.base_dir / 'sources'
        files = list(self._collect_files(source_dir))
        if not files:
            print("Файлы для обработки не найдены")
            return

        file_hashes = self._get_file_hashes(files)
        changed_files = self._get_changed_files(file_hashes)

        if not changed_files:
            print("Индекс актуален, обновление не требуется")
            return

        print(f"Найдено {len(changed_files)} файлов для обработки...")
        self.index = faiss.IndexFlatIP(384)
        
        start_time = time.time()
        with ThreadPoolExecutor(max_workers=2) as executor:
            futures = []
            for i, file in enumerate(changed_files, 1):
                futures.append(executor.submit(self._process_file, file, i, len(changed_files)))
                if i % 5 == 0:
                    self._free_memory()

            for i, future in enumerate(futures, 1):
                try:
                    future.result()
                    self._print_progress(i, len(changed_files))
                except Exception as e:
                    print(f"\nОшибка: {str(e)}")

        self._save_index()
        self._save_file_hashes(file_hashes)
        print(f"\nИндексация завершена за {time.time()-start_time:.1f} сек.")
        print(f"Всего чанков: {len(self.metadata)}")

    def _smart_chunking(self, code: str) -> list:
        """Улучшенное разделение с проверками"""
        if not code or not code.strip():
            return []
            
        lines = code.split('\n')
        chunks = []
        pos = 0
        max_lines = 5000  # Максимум 5000 строк на файл
        
        while pos < len(lines) and pos < max_lines:
            end = min(pos + self.chunk_size, len(lines), max_lines)
            chunk = lines[pos:end]
            
            # Поиск границ с защитой от бесконечного цикла
            boundary = self._find_logical_boundary(chunk)
            if boundary > 0 and boundary < len(chunk):
                actual_end = pos + boundary
                chunk = lines[pos:actual_end]
                pos = actual_end - self.overlap
            else:
                pos = end - self.overlap
                
            if chunk:  # Добавляем только непустые чанки
                chunks.append('\n'.join(chunk))
            
            if pos < 0:
                break

        return chunks

    def _find_logical_boundary(self, chunk: list) -> int:
        """Поиск границ блоков кода"""
        stack = []
        for i, line in enumerate(chunk):
            stripped = line.strip()
            if stripped.startswith(('class ', 'struct ', 'namespace ', 'void ', 'int ', 'bool ')):
                if not stack:
                    return i
            if '{' in line:
                stack.append(i)
            if '}' in line:
                if stack:
                    stack.pop()
                    if not stack:
                        return i+1
        return 0


    def _free_memory(self):
        """Улучшенное освобождение памяти"""
        if self.emb_model is not None:
            del self.emb_model
            self.emb_model = None
        gc.collect()
        time.sleep(0.5)  # Даем время на освобождение памяти



    def _process_file(self, file_path: Path, file_num: int, total_files: int):
        try:
            # Проверка размера файла перед обработкой
            if file_path.stat().st_size > 1 * 1024 * 1024:  # 1MB
                print(f"⚠️ Файл {file_path.name} слишком большой, пропускаем")
                return

            print(f"\n📁 Обработка ({file_num}/{total_files}): {file_path.name}")
            
            with open(file_path, 'r', encoding='utf-8', errors='replace') as f:
                content = f.read()
                chunks = self._smart_chunking(content)
                
                if not chunks or len(chunks) == 0:
                    print(f"⚠️ Файл {file_path.name} не содержит валидных чанков")
                    return
                
                batch_size = 5  # Уменьшаем размер батча
                for i in range(0, len(chunks), batch_size):
                    batch = chunks[i:i+batch_size]
                    
                    # Фильтрация пустых чанков
                    valid_chunks = [chunk for chunk in batch if chunk.strip()]
                    if not valid_chunks:
                        continue
                    
                    # Обработка с контролем памяти
                    try:
                        embs = self.emb_model.encode(valid_chunks, convert_to_numpy=True)
                        self.index.add(embs)
                        
                        # Сохраняем только первые 100 символов
                        self.metadata.extend({
                            'path': str(file_path),
                            'content': chunk[:100],
                            'hash': md5(chunk.encode()).hexdigest()
                        } for chunk in valid_chunks)
                        
                    except Exception as e:
                        print(f"⚠️ Ошибка обработки батча: {str(e)}")
                    finally:
                        del embs, valid_chunks, batch
                        gc.collect()

        except Exception as e:
            print(f"⚠️ Критическая ошибка в {file_path.name}: {str(e)}")
        finally:
            del content, chunks
            gc.collect()


    def ask(self, question: str) -> str:
        """Обработка запроса"""
        try:
            question_emb = self.emb_model.encode([question])
            distances, indices = self.index.search(question_emb, 2)
            
            # Исправление: проверка валидности индексов
            valid_indices = [i for i in indices[0] if 0 <= i < len(self.metadata)]
            
            if not valid_indices:
                return "Релевантная информация не найдена"
                
            context = "\n".join(
                f"[[Файл: {self.metadata[i]['path']}]]\n{self.metadata[i]['content']}"
                for i in valid_indices
            )
        
            prompt = (
                "Контекст:\n{context}\n\n"
                "Вопрос: {question}\n"
                "Ответ (кратко, 1-2 предложения):"
            ).format(context=context, question=question)

            response = self.ollama.generate(
                model='llama3.2',
                prompt=prompt,
                options={'temperature': 0.3, 'num_predict': 100}
            )
            return response['response']
        
        except Exception as e:
            return f"Ошибка: {str(e)}"


if __name__ == "__main__":
    try:
        rag = OptimizedCppRAG()
        print("🔄 Начало индексации...")
        rag.index_codebase()
        
        # Добавить этот блок для обработки запросов
        while True:
            question = input("\n❓ Вопрос (или 'exit'): ").strip()
            if question.lower() in ('exit', 'quit'):
                break
                
            start = time.time()
            print("\n🔍 Поиск...")
            response = rag.ask(question)  # Убедитесь что метод ask() существует
            print(f"\n⏱ {time.time()-start:.1f} сек.\n💡 Ответ:\n{response}")
            
    except KeyboardInterrupt:
        print("\n🛑 Завершение работы...")
    finally:
        rag._free_memory()
        gc.collect()