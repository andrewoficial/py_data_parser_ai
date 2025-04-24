import time
import re
import os
from datetime import datetime
from typing import Dict
from datetime import datetime
import json
from prj_config import Config

class CodeChunker:
    def __init__(self, chunk_size=256, overlap=64):
        print("Create CodeChunker instanse")
        self.chunk_size = chunk_size
        self.overlap = overlap
        self.last_position = 0
        self.last_file = "♥ iNiTiAl _ VaLuE _ ♥"
        self.last_method = ""
        self.last_method_start_position = 0
        self.last_method_end_position = 0
        self.call_for_same_file_counter = 0
        self.same_file = False #false только с большой буквы
        print("[same_file debug] set to false on onotal startup")
        self.current_chunk = None #null
        print(f"\n[CodeChunker Init] Chunk size: {chunk_size}, Overlap: {overlap}")

    def chunk(self, code: str, file_name) -> list:
        """Вызывается в файле indexing_manager.py в методе _process_file_content"""
        start_time = time.time()
        print("\n[CodeChunker Start] Processing code in file " + str(file_name))
        
        if not code.strip():
            print("[CodeChunker Error] Empty input")
            return []
            
        lines = code.split('\n')
        chunks = []
        pos = 0
        
        while pos < len(lines):
            end = min(pos + self.chunk_size, len(lines))
            chunk = lines[pos:end]
            
            # Логирование чанка
            chunk_info = f"CodeChunker Chunk {len(chunks)+1}: lines {pos+1}-{end}"
            if len(chunk) < self.chunk_size:
                chunk_info += f" ({len(chunk)} lines)"
            print(chunk_info)
            
            # Поиск границ
            boundary, _ = self._find_boundary(chunk, file_name)
            
            if boundary > 0:
                actual_end = pos + boundary
                pos = max(actual_end - self.overlap, pos + 1)
                chunks.append('\n'.join(lines[pos:actual_end]))
            else:
                chunks.append('\n'.join(chunk))
                pos = end
        
        print(f"\n[CodeChunker Done] Created {len(chunks)} chunks in {time.time()-start_time:.2f}s")
        return chunks

    def _write_boundary_log(self, filename, entries):
        log_dir = r"D:\AI\RagLoangGas\logs"
        os.makedirs(log_dir, exist_ok=True)
        
        log_file = os.path.join(log_dir, "boundary_analysis.log")
        timestamp = datetime.now().strftime("%Y-%m-%d %H:%M:%S")
        
        with open(log_file, "a", encoding="utf-8") as f:
            f.write(f"\n=== File: {filename} ({timestamp}) ===\n")
            if entries:
                f.write("\n".join(entries) + "\n")
            else:
                f.write("No methods found in this chunk\n")

    def generate_chunk_log(self, chunks, source_filename):
        log_dir = r"D:\AI\RagLoangGas\logs"
        os.makedirs(log_dir, exist_ok=True)
        
        log_file = os.path.join(log_dir, "chunk_analysis.log")
        timestamp = datetime.now().strftime("%Y-%m-%d %H:%M:%S")
        
        with open(log_file, "a", encoding="utf-8") as f:
            f.write(f"\n\n=== File: {source_filename} ({timestamp}) ===\n")
            
            for i, chunk in enumerate(chunks, 1):
                functions = self._analyze_chunk(chunk)
                f.write(f"\n[Chunk {i}] Size: {len(chunk.splitlines())} lines\n")
                
                if functions:
                    f.write("Contains:\n")
                    for func in functions:
                        f.write(f"  • {func}\n")
                else:
                    f.write("No recognized functions/blocks\n")

    def _analyze_chunk(self, chunk):
        functions = []
        patterns = {
            'function': re.compile(r'^\s*(?:virtual\s+)?[\w\s*&]+::\s*~?\w+\s*\([^)]*\)\s*{'),
            'static_block': re.compile(r'^\s*static\s*{'),
            'namespace': re.compile(r'^\s*namespace\s+\w+\s*{')
        }
        
        for line in chunk.split('\n'):
            stripped = line.strip()
            for category, pattern in patterns.items():
                if pattern.match(stripped):
                    clean_line = stripped.split('{')[0].strip()
                    functions.append(f"{category}: {clean_line}")
                    break
                    
        return functions

    def _find_boundary(self, chunk, source_filename):
        #ToDo создать объект MyMetaInfo metaInfo; 
        # myMetaInfo поле MetaInfo (подкласс) fileName, curentPartNum, totalParts
        # myMetaInfo поле HashMap<Stinrg functionName, String functionBody>
        # myMetaInfo поле (служебное приватное) текущий размер чанка, дата создания, лимиты по размерам при создании
        #
        self.call_for_same_file_counter = self.call_for_same_file_counter + 1
        log_entries = []
        method_start_line = -1
        method_name = ""
        meta = ChunkMetaInfo(
            file_name=source_filename,
            current_part=self.call_for_same_file_counter,
            total_parts=10
        )
        self.same_file = True
        print("[same_file debug] set to true on runing _find_boundary")
        print("[same_file debug] and now it is" + str(self.same_file))

        if self.last_file is None:
            print(f"! [same_file debug] Reset flag same_file because last_file is null")
        
        if source_filename is None:
            print(f"! [same_file debug] Reset flag same_file because source_filename is null")
        

        if self.last_file.lower() != source_filename.lower():
            print("! [same_file debug] Reset flag same_file becourse " + self.last_file + " NE RAVNY " + source_filename)
            self.last_file = source_filename
            self.last_method = ""
            self.last_position = 0
            self.last_method_start_position = 0
            self.last_method_end_position = 0
            self.call_for_same_file_counter = 0;
            self.current_chunk = Chunk(meta)
            self.same_file = False
        
        for i, line in enumerate(chunk):
            stripped = line.strip()
            
            # Исхожу из того, что код хорошо форматирован (определяю, что работаю с границами класса или метода)
            # Проверяю, что строка не начинается с пробела или табуляции
            notSubLevel = not (line.startswith(' ') or line.startswith('\t'))

            # Поиск открывающей скобки
            if '{' in line and notSubLevel:
                open_brace_pos = line.find('{')



                # Вариант 1: Метод и скобка в одной строке
                if open_brace_pos > 0:
                    context = line[:open_brace_pos].strip()
                    if context:
                        log_entries.append(f"Found block start at line {i+1} (curr line) named '{context}'")
                        self.last_method = context
                        last_method_start_position = i+1
                        

                # Вариант 2: Метод и скобка в разных строчках
                elif i > 0:
                    prev_line = chunk[i-1].strip()
                    if prev_line:
                        log_entries.append(f"Found block start at line {i} (prev line) named '{prev_line}'")
                        self.last_method = prev_line
                        self.last_method_start_position = i

            # Поиск закрывающей скобки
            if '}' in line and notSubLevel:
                log_entries.append(f"Found block end at line {i} (previous line) named '{self.last_method}'")
                log_entries.append(f"This is same file? '{self.same_file}' /n")
                print("[same_file debug] on close tag flag state is " + str(self.same_file)) #  типа ValueOf
                self.last_method_end_position = i

                # тело метода
                method_body = chunk[self.last_method_start_position:self.last_method_end_position + 1]
                method_body_text = '\n'.join(method_body)

                self.current_chunk.add_function(
                    self.last_method,  # Имя метода
                    method_body_text   # Тело метода
                )
            
            if not self.same_file: #ToDo чего-то не сбрасывает.... Определяет изменение файла вообще верно
                self.same_file = True
                print("[same_file debug] Set same_file to true in for-loop on i position" + str(i))
                print("[same_file debug] and now flag state is " + str(self.same_file))
                

        # Логирование статистики
        # Добавил верный подсчёт строк
        if self.last_position == 0:
            self.last_position = chunk.count('\n')
        else:
            subchunk = chunk[-(self.overlap):]  # Используем срез для получения подстроки
            line_counters = subchunk.count('\n')  # Считаем количество символов новой строки
            self.last_position += line_counters
        
        log_entries.append(f"\nTotal lines processed: {self.last_position}")
        log_entries.append(f"Total brackets found: {len(log_entries)//3}")
        log_entries.append(json.dumps(self.current_chunk.get_debug_info(), indent=2)) # Это конечно кек
        self._write_boundary_log(source_filename, log_entries)


        return 0, []

class ChunkMetaInfo:
    def __init__(self, file_name: str, current_part: int = 1, total_parts: int = 1):
        self.config = Config()
        self.file_name = file_name          # Имя исходного файла
        self.current_part = current_part    # Номер текущей части (для разбитых объектов)
        self.total_parts = total_parts      # Общее количество частей
        self.created_at = datetime.now()    # Дата создания
        self.size_limit = self.config.CHUNK_SIZE   # Лимит размера в символах

    def update_part_info(self, current: int, total: int):
        """Обновление информации о частях"""
        self.current_part = current
        self.total_parts = total

    def to_dict(self) -> Dict:
        """Сериализация в словарь"""
        return {
            'file': self.file_name,
            'part': f"{self.current_part}/{self.total_parts}",
            'created': self.created_at.isoformat(),
            'size_limit': self.size_limit
        }

class Chunk:
    def __init__(self, meta: ChunkMetaInfo):
        self.meta = meta                      # Метаинформация
        self.functions: Dict[str, str] = {}   # Словарь функция-тело
        self._internal = {                    # Служебная информация
            'raw_size': 0,
            'is_valid': True
        }
    
    def add_function(self, name: str, body: str):
        """Добавление функции в чанк"""
        self.functions[name] = body
        self._update_size(len(body))
    
    def _update_size(self, added_size: int):
        """Обновление служебной информации"""
        self._internal['raw_size'] += added_size
        if self._internal['raw_size'] > self.meta.size_limit:
            self._internal['is_valid'] = False
    
    def is_valid(self) -> bool:
        """Проверка соответствия лимитам"""
        return self._internal['is_valid']
    
    def get_debug_info(self) -> Dict:
        """Отладочная информация"""
        return {
            'meta': self.meta.to_dict(),
            'functions_count': len(self.functions),
            'total_size': self._internal['raw_size'],
            'is_valid': self._internal['is_valid']
        }