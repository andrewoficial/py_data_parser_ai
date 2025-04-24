# prj_processors/indexing_manager.py
import gc
from pathlib import Path
from concurrent.futures import ThreadPoolExecutor
from hashlib import md5
import time

class IndexingManager:
    def __init__(self, file_processor, faiss_manager, embedder, chunker):
        self.file_processor = file_processor
        self.faiss = faiss_manager
        self.embedder = embedder
        self.chunker = chunker

    def index_codebase(self):
        """Основной метод для индексации кодовой базы"""
        print("[Indexing] Checking for existing vector index...")
        if self.faiss.load_index():
            print("[Indexing] ✓ Existing index loaded successfully")
            return

        print("[Indexing] No existing index found, creating new one")
        
        files = self._collect_and_process_files()
        if not files:
            return

        changed_files = self._get_changed_files(files)
        if not changed_files:
            return

        self._process_files(changed_files)
        self._save_results()

    def _collect_and_process_files(self):
        """Сбор и обработка файлов"""
        print("[Indexing] Scanning source directory for files...")
        files = list(self.file_processor.collect_files())
        print(f"[Indexing] Found {len(files)} files to process")
        return files

    def _get_changed_files(self, files):
        """Получение измененных файлов"""
        print("[Indexing] Calculating file hashes for change detection...")
        file_hashes = self.file_processor.get_file_hashes(files)
        
        print("[Indexing] Identifying changed files...")
        changed_files = self.file_processor.get_changed_files(file_hashes)
        print(f"[Indexing] {len(changed_files)} files need processing")
        return changed_files

    def _process_files(self, changed_files):
        """Однопоточная обработка файлов"""
        print("[Indexing] Initializing FAISS index with 384 dimensions...")
        self.faiss.initialize_index()
        
        print("[Indexing] Starting file processing...")
        for i, file in enumerate(changed_files, 1):
            print(f"  → Processing file {i}/{len(changed_files)}: {file.name}")
            try:
                self.process_single_file(file)
            except Exception as e:
                print(f"  ! Error processing {file.name}: {str(e)}")

    def _save_results(self):
        """Сохранение результатов индексации"""
        print("[Indexing] Saving new index and metadata...")
        self.faiss.save_index()
        print("[Indexing] Indexing process completed successfully!\n")

    def process_single_file(self, file_path: Path):
        """Обработка одного файла"""
        try:
            print(f"    [Processing] {file_path.name}")
            start_time = time.time()
            
            with open(file_path, 'r', encoding='utf-8', errors='replace') as f:
                content = f.read()
                self._process_file_content(file_path, content)
                

        except Exception as e:
            print(f"    ! Error processing {file_path.name}: {str(e)}")
        finally:
            gc.collect()

    def _process_file_content(self, file_path: Path, content: str):
        """Обработка содержимого файла"""
        #Чанкование начинается тут
        chunks = self.chunker.chunk(content, file_path.name)
        self.chunker.generate_chunk_log(chunks, file_path.name)
        if not chunks:
            print(f"    ! No chunks found in {file_path.name}")
            return

        print(f"    • Chunks created: {len(chunks)}")
        batch_size = 5
        total_vectors = 0
        
        for i in range(0, len(chunks), batch_size):
            total_vectors += self._process_batch(file_path, chunks[i:i+batch_size])

        print(f"     Total vectors added: {total_vectors}")

    def _process_batch(self, file_path: Path, batch: list):
        """Обработка батча чанков"""
        valid_chunks = [chunk for chunk in batch if chunk.strip()]
        if not valid_chunks:
            return 0

        vectors = self.embedder.encode(valid_chunks)
        self._store_vectors(file_path, valid_chunks, vectors)
        return len(vectors)

    def _store_vectors(self, file_path: Path, chunks: list, vectors):
        """Сохранение векторов и метаданных"""
        self.faiss.index.add(vectors)
        self.faiss.metadata.extend({
            'path': str(file_path),
            'content': chunk[:200],
            'hash': md5(chunk.encode()).hexdigest()
        } for chunk in chunks)
        
        # Очистка памяти
        del vectors, chunks
        gc.collect()