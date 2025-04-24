from prj_config import Config
from prj_processors.file_processor import FileProcessor
from prj_processors.indexing_manager import IndexingManager
from prj_vector_db.faiss_manager import FaissManager
from prj_models.embedding_model import EmbeddingModel
from prj_chat.llm_handler import LLMHandler
from prj_processors.chunker import CodeChunker
from concurrent.futures import ThreadPoolExecutor
from pathlib import Path
import gc
import shutil
import os

class RagSystem:
    def __init__(self):
        print("[Init] Loading configuration...")
        self.config = Config()
        
        print("[Init] Initializing file processor with configuration...")
        self.file_processor = FileProcessor(self.config)
        
        print("[Init] Setting up FAISS vector database...")
        self.faiss = FaissManager(self.config)
        
        print("[Init] Loading embedding model (all-MiniLM-L6-v2)...")
        self.embedder = EmbeddingModel()
        
        print(f"[Init] Configuring code chunker (chunk_size={self.config.CHUNK_SIZE}, overlap={self.config.OVERLAP})")
        self.chunker = CodeChunker(
            chunk_size=self.config.CHUNK_SIZE,
            overlap=self.config.OVERLAP
        )

        print("[Init] Loading LLMHandler...")
        self.llm_handler = LLMHandler(self.embedder, self.faiss)

        self.indexing_manager = IndexingManager(
            file_processor=self.file_processor,
            faiss_manager=self.faiss,
            embedder=self.embedder,
            chunker=self.chunker
        )

        print("[Init] System initialization complete!\n")

    def index_codebase(self):
        self.indexing_manager.index_codebase()

    @staticmethod
    def clean_start():
        """Удаляет папки vector_store и logs с их содержимым"""
        paths_to_remove = [
            r"D:\AI\RagLoangGas\vector_store",
            r"D:\AI\RagLoangGas\logs"
        ]
        
        for path in paths_to_remove:
            try:
                if os.path.exists(path):
                    shutil.rmtree(path)
                    print(f"Successfully removed: {path}")
                else:
                    print(f"Path does not exist: {path}")
            except Exception as e:
                print(f"Error removing {path}: {str(e)}")

if __name__ == "__main__":
    print("=== Starting RAG System ===")
    print("[Main] Run preparing...")
    RagSystem.clean_start()
    print("[Main] Initializing system components...")
    system = RagSystem()
    print("\n[Main] Starting codebase indexing...")
    system.index_codebase()
    print("\n=== System Ready ===")
    print("\n[Main] Starting session...")
    system.llm_handler.interactive_loop()
    # Очистка ресурсов
    print("\n[Main] Release resources...")
    system.embedder.release_memory()
    gc.collect()
    print("\n[Main] Exit...............")