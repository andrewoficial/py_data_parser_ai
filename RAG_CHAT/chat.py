import os
import faiss
import requests
import json
from sentence_transformers import SentenceTransformer

# Пути к данным
DATA_PATH = "D:/AI/RAG/TextToVectors/QandA_summary.txt"
VECTOR_DB_PATH = "D:/AI/RAG/TextToVectors/faiss_index"
OLLAMA_API_URL = "http://localhost:11434/api/generate"
MAX_LENGTH = 256  # Максимальная длина текста (в символах)

# Загрузка модели для эмбеддингов
embedding_model = SentenceTransformer('all-MiniLM-L6-v2')

# Функция загрузки данных из файла
def load_data(file_path):
    questions, answers = [], []
    warnings = []
    with open(file_path, "r", encoding="utf-8") as file:
        for line in file:
            if line.startswith("Q:"):
                question = line.strip().replace("Q:", "").strip()
                print(f"Question {len(questions)}: {line[2:].strip()}")
                if len(question) > MAX_LENGTH:
                    warnings.append(f"Вопрос превышает {MAX_LENGTH} символов: {question}")
                    print(f"    Вопрос превышает {MAX_LENGTH} символов: {question}")
                questions.append(question)
                
            elif line.startswith("A:"):
                answer = line.strip().replace("A:", "").strip()
                print(f"Answer {len(answers)}: {line[2:].strip()}")
                if len(answer) > MAX_LENGTH:
                    warnings.append(f"    Ответ превышает {MAX_LENGTH} символов: {answer}")
                answers.append(answer)
                
    # Вывод предупреждений
    if warnings:
        print("\n⚠️ Предупреждения о длине данных:")
        for warn in warnings:
            print(warn)
    else:
        print("\n✅ Все вопросы и ответы в пределах допустимой длины.")
    
    return questions, answers

def print_embedding_info(embedding_model):
    dummy_text = ["тест"]
    embeddings = embedding_model.encode(dummy_text)
    print(f"\n✅ Модель создает эмбеддинги размерностью: {embeddings.shape[1]}")

# Создание/загрузка векторного индекса
def create_or_load_index(questions, embeddings, index_path):
    embedding_size = embeddings.shape[1]
    index = None

    if not os.path.exists(index_path):
        print("Создание нового индекса HNSW...")
        index = faiss.IndexHNSWFlat(embedding_size, 96)  # 32 - число соседей (M-параметр)
        index.hnsw.efConstruction = 600  # Параметр для качества построения индекса 200
        index.add(embeddings)
        faiss.write_index(index, index_path)
    else:
        print("Загрузка существующего индекса HNSW...")
        index = faiss.read_index(index_path)

        # Проверяем, совпадает ли размер индекса с количеством вопросов
        if index.ntotal != len(questions):
            print("Размер индекса не совпадает с количеством вопросов. Пересоздание индекса HNSW...")
            index = faiss.IndexHNSWFlat(embedding_size, 96)
            index.hnsw.efConstruction = 600
            index.add(embeddings)
            faiss.write_index(index, index_path)
    index.hnsw.efSearch = 400 #50
    return index

# Генерация текста через Ollama
#def generate_response(prompt, model_name="llama3.1"):
def generate_response(prompt, model_name="deepseek-r1"):
    url = "http://127.0.0.1:11434/api/generate"
    headers = {"Content-Type": "application/json"}
    data = {"model": model_name, "prompt": prompt}
    response_text = ""  # Для хранения полного ответа

    try:
        with requests.post(OLLAMA_API_URL, headers=headers, json=data, stream=True) as response:
            response.raise_for_status()  # Проверка на ошибки
            
            # Обработка потокового ответа
            for line in response.iter_lines(decode_unicode=True):
                if line:
                    try:
                        part = json.loads(line)
                        response_text += part.get("response", "")  # Добавляем фрагмент ответа
                        if part.get("done", False):  # Если done, завершить сбор
                            break
                    except json.JSONDecodeError:
                        print("Ошибка обработки JSON:", line)
        
        return response_text if response_text else "Ошибка: пустой ответ."

    except requests.exceptions.RequestException as e:
        return f"Ошибка при запросе: {e}"

# Главная функция
def main():
    # Загрузка данных
    questions, answers = load_data(DATA_PATH)

    # Проверка количества считанных данных
    num_questions = len(questions)
    num_answers = len(answers)
    print(f"\nЗагружено {num_questions} вопросов и {num_answers} ответов из файла.\n")

    # Создание эмбеддингов
    print("Создание эмбеддингов...")
    embeddings = embedding_model.encode(questions)
    print(f"Создано {len(embeddings)} эмбеддингов с размерностью {embeddings.shape[1]}")

    # Создание или загрузка индекса
    index = create_or_load_index(questions, embeddings, VECTOR_DB_PATH)

    # Проверка размерности эмбеддингов
    print_embedding_info(embedding_model)
    print("Введите вопрос (или 'exit' для выхода)")
    while True:
        query = input("\nВы: ")
        if query.lower() == "exit":
            print("Завершение работы программы.")
            break

        # Вывод запроса
        #print(f"Вы: {query}")

        # Создание эмбеддинга для запроса
        query_embedding = embedding_model.encode([query])

        # Проверяем количество данных
        k = min(3, len(questions))
        if k == 0:
            print("Нет данных для поиска.")
            return

        distances, indices = index.search(query_embedding, k)
        indices = [i for i in indices[0] if 0 <= i < len(questions)]  # Фильтруем индексы

        if not indices:
            print("Нет релевантных результатов.")
            continue

        # Формирование ответа
        retrieved_texts = [
            f"Q: {questions[i]} A: {answers[i]}"
            for i in indices
        ]
        prompt = f"На основе данных:\n{retrieved_texts}\nОтветь на вопрос: {query}"
        response = generate_response(prompt)
        print(f"AI: {response}")
        

if __name__ == "__main__":
    main()
