from pathlib import Path

class Config:
    def __init__(self):
        # Инициализация параметров
        self.BASE_DIR = Path(r'D:\AI\RagLoangGas')
        self.SOURCES_DIR = self.BASE_DIR / 'sources'
        self.VECTOR_STORE_DIR = self.BASE_DIR / 'vector_store'
        self.CACHE_PATH = self.BASE_DIR / 'file_hashes.json'
        
        self.CHUNK_SIZE = 256
        self.OVERLAP = 32
        self.IGNORE_DIRS = {'build', 'test', 'thirdparty'}
        self.FILE_EXTENSIONS = {'cpp', 'hpp', 'h', 'cc', 'cxx'}
        self.MAX_FILE_SIZE = 2 * 1024 * 1024  # 2MB

        # Вывод параметров конфигурации
        self.print_config()

    def print_config(self):
        print("\n=== Configuration Parameters ===")
        print(f"[Paths]")
        print(f"  • BASE_DIR: {self.BASE_DIR}")
        print(f"  • SOURCES_DIR: {self.SOURCES_DIR}")
        print(f"  • VECTOR_STORE_DIR: {self.VECTOR_STORE_DIR}")
        print(f"  • CACHE_PATH: {self.CACHE_PATH}")
        
        print("\n[Processing Parameters]")
        print(f"  • CHUNK_SIZE: {self.CHUNK_SIZE} characters")
        print(f"  • OVERLAP: {self.OVERLAP} characters")
        print(f"  • MAX_FILE_SIZE: {self.MAX_FILE_SIZE/1024/1024:.1f} MB")
        
        print("\n[Filtering Parameters]")
        print(f"  • IGNORE_DIRS: {', '.join(self.IGNORE_DIRS)}")
        print(f"  • FILE_EXTENSIONS: {', '.join(self.FILE_EXTENSIONS)}")
        print("=" * 32 + "\n")