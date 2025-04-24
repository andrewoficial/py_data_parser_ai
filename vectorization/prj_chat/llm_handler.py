# llm_handler.py
import time
from typing import Optional
from ollama import Client

class LLMHandler:
    def __init__(self, 
                 emb_model, 
                 faiss_manager, 
                 model_name: str = 'llama3.2',
                 temperature: float = 0.3,
                 max_tokens: int = 100):
        """
        Инициализация обработчика LLM
        
        :param emb_model: Модель для создания эмбеддингов
        :param faiss_manager: Менеджер векторной базы данных
        :param model_name: Название LLM модели
        :param temperature: Параметр температуры для генерации
        :param max_tokens: Максимальное количество токенов в ответе
        """
        self.emb_model = emb_model
        self.faiss = faiss_manager
        self.client = Client(host='http://localhost:11434')
        
        self.model_name = model_name
        self.temperature = temperature
        self.max_tokens = max_tokens
        
        print("\n[LLMHandler] Initializing language model handler")
        print(f"  • Model: {self.model_name}")
        print(f"  • Temperature: {self.temperature}")
        print(f"  • Max tokens: {self.max_tokens}\n")

    def generate_response(self, question: str) -> str:
        """
        Обработка пользовательского запроса с полным циклом RAG
        
        :param question: Входной вопрос пользователя
        :return: Ответ модели
        """
        try:
            start_time = time.time()
            
            # Этап 1: Поиск релевантного контекста
            print(f"[RAG Pipeline] Processing question: '{question}'")
            context = self._get_context(question)
            
            if not context:
                return "Релевантная информация не найдена в кодовой базе"
            
            # Этап 2: Формирование промпта
            prompt = self._build_prompt(question, context)
            
            # Этап 3: Генерация ответа
            print("[Generation] Sending request to LLM...")
            response = self.client.generate(
                model=self.model_name,
                prompt=prompt,
                options={
                    'temperature': self.temperature,
                    'num_predict': self.max_tokens
                }
            )
            
            # Логирование результатов
            print(f"[Generation] Response received ({time.time()-start_time:.1f}s)")
            print(f"  • Tokens used: {response.get('eval_count', 'N/A')}")
            
            return response['response']
            
        except Exception as e:
            print(f"[Error] Failed to generate response: {str(e)}")
            return f"Ошибка при обработке запроса: {str(e)}"

    def _get_context(self, question: str) -> Optional[str]:
        """Поиск релевантного контекста в векторной базе"""
        try:
            print("[Retrieval] Encoding question...")
            start_time = time.time()
            
            question_emb = self.emb_model.encode([question])
            print(f"  ✓ Question encoded ({time.time()-start_time:.2f}s)")
            
            print("[Retrieval] Searching in vector database...")
            distances, indices = self.faiss.index.search(question_emb, 3)
            
            valid_indices = [i for i in indices[0] if 0 <= i < len(self.faiss.metadata)]
            if not valid_indices:
                print("  ✗ No relevant context found")
                return None
                
            print(f"  ✓ Found {len(valid_indices)} relevant chunks")
            return "\n\n".join(
                f"File: {self.faiss.metadata[i]['path']}\nContent:\n{self.faiss.metadata[i]['content']}"
                for i in valid_indices
            )
            
        except Exception as e:
            print(f"[Retrieval Error] {str(e)}")
            return None

    def _build_prompt(self, question: str, context: str) -> str:
        """Формирование промпта для LLM"""
        print("[Prompt Engineering] Building prompt...")
        
        prompt_template = (
            "Context from codebase:\n{context}\n\n"
            "Answer the question based on the context above.\n"
            "Question: {question}\n"
            "Answer (be concise, 2-3 sentences):"
        )
        
        return prompt_template.format(context=context, question=question)

    def interactive_loop(self):
        """Интерактивный режим вопрос-ответ"""
        print("\n=== Interactive Mode ===")
        while True:
            try:
                question = input("\n❓ Your question (type 'exit' to quit): ").strip()
                if question.lower() in ('exit', 'quit'):
                    break
                
                start_time = time.time()
                print("\n🔍 Processing...")
                response = self.generate_response(question)
                
                print(f"\n⏱ Response time: {time.time()-start_time:.1f}s")
                print(f"💡 Answer:\n{'-'*40}\n{response}\n{'-'*40}")
                
            except KeyboardInterrupt:
                print("\n🛑 Operation cancelled by user")
                break
                
        print("\n=== Session Ended ===")