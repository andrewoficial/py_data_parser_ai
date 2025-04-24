import faiss
import json
from pathlib import Path
from prj_config import Config
import time

class FaissManager:
    def __init__(self, config: Config):
        self.config = config
        self.index = None
        self.metadata = []
        
        print("\n[FaissManager] Initializing vector store manager")
        print(f"  • Vector store directory: {self.config.VECTOR_STORE_DIR}")
        print(f"  • Index file: {self.config.VECTOR_STORE_DIR / 'index.faiss'}")
        print(f"  • Metadata file: {self.config.VECTOR_STORE_DIR / 'metadata.json'}\n")

    def initialize_index(self, dimension=384):
        print(f"[IndexInit] Creating new FAISS index with dimension {dimension}")
        start_time = time.time()
        
        self.index = faiss.IndexFlatIP(dimension)
        self.metadata = []
        
        print(f"  ✓ Index created successfully")
        print(f"  Time elapsed: {time.time() - start_time:.2f} seconds\n")

    def save_index(self):
        print("[IndexSave] Saving index and metadata...")
        start_time = time.time()
        
        # Создание директории, если не существует
        self.config.VECTOR_STORE_DIR.mkdir(exist_ok=True)
        print(f"  • Directory ready: {self.config.VECTOR_STORE_DIR}")
        
        # Сохранение индекса
        index_path = str(self.config.VECTOR_STORE_DIR / 'index.faiss')
        faiss.write_index(self.index, index_path)
        print(f"  ✓ Index saved: {index_path}")
        
        # Сохранение метаданных
        metadata_path = self.config.VECTOR_STORE_DIR / 'metadata.json'
        with open(metadata_path, 'w') as f:
            json.dump(self.metadata, f)
        print(f"  ✓ Metadata saved: {metadata_path}")
        
        print(f"  Total vectors stored: {len(self.metadata)}")
        print(f"  Time elapsed: {time.time() - start_time:.2f} seconds\n")

    def load_index(self):
        print("[IndexLoad] Attempting to load existing index...")
        start_time = time.time()
        
        index_file = self.config.VECTOR_STORE_DIR / 'index.faiss'
        metadata_file = self.config.VECTOR_STORE_DIR / 'metadata.json'
        
        if not index_file.exists():
            print("  ✗ Index file not found")
            return False
            
        if not metadata_file.exists():
            print("  ✗ Metadata file not found")
            return False
            
        try:
            # Загрузка индекса
            self.index = faiss.read_index(str(index_file))
            print(f"  ✓ Index loaded: {index_file}")
            
            # Загрузка метаданных
            with open(metadata_file, 'r') as f:
                self.metadata = json.load(f)
            print(f"  ✓ Metadata loaded: {metadata_file}")
            
            print(f"  Total vectors loaded: {len(self.metadata)}")
            print(f"  Time elapsed: {time.time() - start_time:.2f} seconds\n")
            return True
            
        except Exception as e:
            print(f"  ✗ Error loading index: {str(e)}")
            return False