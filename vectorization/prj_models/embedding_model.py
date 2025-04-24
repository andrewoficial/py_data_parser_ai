from sentence_transformers import SentenceTransformer
import gc
import time

class EmbeddingModel:
    def __init__(self, model_name='all-MiniLM-L6-v2'):
        print("\n[EmbeddingModel] Initializing embedding model...")
        self.model = None
        self.model_name = model_name
        self.load_model(model_name)
        print(f"[EmbeddingModel] ✓ Model '{model_name}' ready\n")

    def load_model(self, model_name):
        print(f"[ModelLoader] Loading model '{model_name}'...")
        start_time = time.time()
        
        if self.model is not None:
            print("  • Existing model detected, releasing memory...")
            self.release_memory()
            
        try:
            self.model = SentenceTransformer(model_name, device='cpu')
            load_time = time.time() - start_time
            print(f"[ModelLoader] Model loaded successfully")
            print(f"[ModelLoader] Time elapsed: {load_time:.2f} seconds")
            
        except Exception as e:
            print(f"  ! Error loading model: {str(e)}")
            raise

    def encode(self, texts):
        if not isinstance(texts, (list, tuple)):
            texts = [texts]
            
        print(f"\n[Encoding in embedding_model.py] Processing {len(texts)} text chunk(s)...")
        start_time = time.time()
        
        try:
            embeddings = self.model.encode(texts, convert_to_numpy=True)
            encode_time = time.time() - start_time
            
            print(f"  Encoding completed")
            print(f"  Time elapsed: {encode_time:.2f} seconds")
            
            return embeddings
            
        except Exception as e:
            print(f"  ! Encoding error: {str(e)}")
            raise

    def release_memory(self):
        print("[Memory] Releasing model resources...")
        start_time = time.time()
        
        if self.model is not None:
            print("  • Deleting model instance...")
            del self.model
            
            print("  • Running garbage collector...")
            gc_start = time.time()
            collected = gc.collect()
            gc_time = time.time() - gc_start
            
            print(f"  Memory released")
            print(f"  Garbage collected: {collected} objects")
            print(f"  GC time: {gc_time:.2f} seconds")
            
        self.model = None
        print(f"  Total time: {time.time() - start_time:.2f} seconds\n")

    def __del__(self):
        print("\n[EmbeddingModel] Cleaning up resources...")
        self.release_memory()