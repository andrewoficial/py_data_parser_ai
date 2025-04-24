import os
import json
import time
import random
import re
import logging
from pathlib import Path
import requests
from tenacity import retry, stop_after_attempt, wait_exponential

# Настройки
OPENROUTER_API_KEY = "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"  # Замените на ваш ключ
INPUT_DIR = Path("D:/AI/vector/output_dir")
OUTPUT_DIR = Path("D:/AI/vector/summarization")
MODEL = "meta-llama/llama-3.1-8b-instruct:free"  # Модель через OpenRouter
# Конфигурация (вынесите в настройки)
OLLAMA_ENDPOINT = "http://127.0.0.1:11434/api/generate"
LOCAL_MODEL_NAME = "llama3.2"  # Измените на вашу модель
PROMPT_TEMPLATE = (
    "Составь на основе юридического документа JSON с полями 'result' (обвинительный или оправдательный) и 'base' (статьи на основании которых вынесено решение с пунктами через запятую)"
    "Исходные данные: "
)

# Настройка логгера
# Изменения в настройке логгера
def setup_logger():
    logger = logging.getLogger('json_processor')
    logger.setLevel(logging.DEBUG)  # Включаем детальное логирование

    # Форматтер
    formatter = logging.Formatter(
        '%(asctime)s - %(name)s - %(levelname)s - %(message)s',
        datefmt='%Y-%m-%d %H:%M:%S'
    )

    # Обработчик для файла
    file_handler = logging.FileHandler(
        filename=OUTPUT_DIR / 'processing.log',
        encoding='utf-8',
        mode='w'  # Перезаписываем файл при каждом запуске
    )
    file_handler.setFormatter(formatter)
    file_handler.setLevel(logging.DEBUG)

    # Обработчик для консоли
    console_handler = logging.StreamHandler()
    console_handler.setFormatter(formatter)
    console_handler.setLevel(logging.INFO)

    # Добавляем обработчики
    logger.addHandler(file_handler)
    logger.addHandler(console_handler)

    return logger

logger = setup_logger()

def process_file(input_path: Path):
    logger.info(f"Начало обработки файла: {input_path.name}")
    
    try:
        # Загрузка данных
        data = load_json_data(input_path)
        if not data:
            logger.warning(f"Файл {input_path.name} пуст или содержит некорректные данные")
            return False

        # Обработка элементов
        processing_results = process_items(data, input_path)
        
        # Сохранение результатов
        save_results(input_path, processing_results)
        
        logger.info(f"Успешно обработано {len(processing_results)} элементов в файле {input_path.name}")
        return True

    except Exception as e:
        logger.error(f"Критическая ошибка при обработке файла {input_path.name}: {str(e)}", exc_info=True)
        return False

def load_json_data(file_path: Path):
    """Загрузка и валидация JSON данных"""
    try:
        with open(file_path, 'r', encoding='utf-8') as f:
            data = json.load(f)
            
        if not isinstance(data, list):
            logger.error(f"Ошибка структуры данных в файле {file_path.name}: ожидался список")
            return None
            
        return data
    except json.JSONDecodeError as e:
        logger.error(f"Ошибка декодирования JSON в файле {file_path.name}: {str(e)}")
        return None
    except Exception as e:
        logger.error(f"Ошибка чтения файла {file_path.name}: {str(e)}")
        return None

def process_items(data: list, input_path: Path) -> list:
    """Обработка элементов файла"""
    results = []
    
    for idx, item in enumerate(data, 1):
        try:
            if not isinstance(item, dict):
                logger.warning(f"Элемент {idx} в файле {input_path.name} не является словарем - пропущен")
                continue

            result = process_single_item(item, idx, input_path)
            if result:
                results.append(result)
                
        except Exception as e:
            logger.error(f"Ошибка обработки элемента {idx} в файле {input_path.name}: {str(e)}", exc_info=True)
            continue
            
    return results

def process_single_item(item: dict, item_num: int, input_path: Path) -> dict:
    """Обработка одного элемента"""
    logger.debug(f"Обработка элемента {item_num} в файле {input_path.name}")
    
    field_name = 'title'
    name_delo = item[field_name]
    logger.warning(f"=============Приступаю к [{name_delo}] в файле {input_path.name}=============")

    field_name = 'Postanovlenie suda apelyacionnoi instancii'
    
    # Проверка наличия необходимого поля
    if field_name not in item:
        logger.warning(f"Элемент {item_num} в файле {input_path.name} не содержит поля '{field_name}' - пропущен")
        return None

    text = item[field_name]
    
    # Валидация текста
    if not validate_text(text, item_num, input_path):
        return None

    # Отправка запроса к ИИ
    start_time = time.time()
    try:
        logger.info(f"Отправка запроса для элемента {item_num} в файле {input_path.name} с текстом {text[:5]}.........")
        #ai_response = send_to_ai(text)
        ai_response = send_to_local_ai(text)

        logger.debug(f"Получен ответ для элемента {item_num}: {ai_response[:100]}...")  # Логируем первые 100 символов
    except Exception as e:
        logger.error(f"Ошибка запроса к ИИ для элемента {item_num}: {str(e)}")
        return None
    
    # Парсинг ответа
    try:
        parsed = parse_ai_response(ai_response)
        parsed['processing_time'] = time.time() - start_time
        parsed['original_text'] = text
        parsed['item_number'] = item_num
        logger.info(f"Успешная обработка элемента {item_num} за {parsed['processing_time']:.2f} сек")
        return parsed
    except Exception as e:
        logger.error(f"Ошибка парсинга ответа для элемента {item_num}: {str(e)}\nОтвет: {ai_response}")
        return None

def validate_text(text: str, item_num: int, input_path: Path) -> bool:
    """Валидация текста перед отправкой"""
    if not text:
        logger.warning(f"Пустой текст в элементе {item_num} файла {input_path.name}")
        return False
    if len(text) < 50:
        logger.warning(f"Слишком короткий текст ({len(text)} символов) в элементе {item_num} файла {input_path.name}")
        return False
    return True

def save_results(input_path: Path, results: list):
    """Сохранение результатов обработки"""
    if not results:
        logger.warning(f"Нет результатов для сохранения в файле {input_path.name}")
        return

    output_path = OUTPUT_DIR / f"{input_path.stem}_summarization.json"
    try:
        with open(output_path, 'w', encoding='utf-8') as f:
            json.dump(results, f, ensure_ascii=False, indent=2)
        logger.info(f"Результаты сохранены в {output_path.name}")
    except Exception as e:
        logger.error(f"Ошибка сохранения результатов для файла {input_path.name}: {str(e)}")

@retry(stop=stop_after_attempt(3), wait=wait_exponential(multiplier=1, min=10, max=60))
def send_to_ai(text: str):
    """Отправка запроса через OpenRouter API с полной предварительной обработкой текста"""
    logger.info("Вызван send_to_ai с текстом длиной: %d символов", len(text))
    
    # 1. Проверка на None и пустую строку
    if text is None:
        logger.error("Получен None вместо текста")
        raise ValueError("Текст не может быть None")
    
    if not text.strip():
        logger.error("Получена пустая строка")
        raise ValueError("Текст не может быть пустым")

    # 2. Очистка текста
    try:
        # Удаление лишних пробелов, переносов и спецсимволов
        cleaned_text = ' '.join(text.strip().split())
        logger.error("Очистка текста от лишних символов")
        cleaned_text = cleaned_text.replace('\u200b', '').replace('\ufeff', '')  # Удаление невидимых символов
        logger.error("Очистка текста от невидимых символов")
        # Проверка на минимальную длину
        if len(cleaned_text) < 50:
            logger.warning("Текст слишком короткий после очистки: %d символов", len(cleaned_text))
            raise ValueError("Текст слишком короткий для анализа")
            
        logger.error("Проверка на максимальную длину")
        # Проверка на максимальную длину
        MAX_LENGTH = 8000  # С запасом от лимита модели
        if len(cleaned_text) > MAX_LENGTH:
            logger.warning("Текст слишком длинный, будет обрезан: %d > %d символов", 
                         len(cleaned_text), MAX_LENGTH)
            cleaned_text = cleaned_text[:MAX_LENGTH] + " [TRUNCATED]"
            
    except Exception as e:
        logger.error("Ошибка обработки текста: %s", str(e))
        raise ValueError(f"Ошибка обработки текста: {str(e)}")

    logger.error("Формирование заголовков")
    # 3. Формирование запроса
    headers = {
        "Authorization": f"Bearer {OPENROUTER_API_KEY}",
        "HTTP-Referer": "https://github.com/legal-ai-processor",
        "X-Title": "Legal AI Processor",
        "Content-Type": "application/json"
    }
    logger.error("Формирование запроса")
    payload = {
        "model": MODEL,
        "messages": [{
            "role": "user",
            "content": PROMPT_TEMPLATE + " {" + cleaned_text + "}"
        }]
    }
    logger.error("Отладки")
    # Логирование для отладки
    #logger.error("=== Запрос к OpenRouter ===")
    #logger.error("Модель: %s", MODEL)
    #logger.error("Длина текста: %d", len(cleaned_text))
    #logger.error("Первые 100 символов: %s", cleaned_text[:100])
    #logger.error("===========================")

    # 4. Отправка запроса
    try:
        start_time = time.time()
        response = requests.post(
            url="https://openrouter.ai/api/v1/chat/completions",
            headers=headers,
            data=json.dumps(payload, ensure_ascii=False),
            timeout=60
        )
        
        logger.error("Время выполнения запроса: %.2f сек", time.time() - start_time)
        response.raise_for_status()
        return handle_api_response(response)
        
    except requests.exceptions.HTTPError as e:
        error_msg = f"HTTP {e.response.status_code}"
        try:
            error_data = e.response.json()
            error_msg += f": {error_data.get('error', {}).get('message', str(error_data))}"
        except:
            error_msg += f": {e.response.text[:200]}"
        logger.error("Ошибка API: %s", error_msg)
        raise Exception(f"API Error: {error_msg}")
        
    except requests.exceptions.Timeout:
        logger.error("Таймаут запроса (60 сек)")
        raise Exception("Превышено время ожидания ответа")
        
    except Exception as e:
        logger.error("Неожиданная ошибка: %s", str(e), exc_info=True)
        raise Exception(f"Unexpected error: {str(e)}")


@retry(stop=stop_after_attempt(3), wait=wait_exponential(multiplier=1, min=2, max=30))
def send_to_local_ai(text: str):
    """Отправка запроса к локальной модели через OLLAMA API"""
    logger.info("Вызван send_to_local_ai с текстом длиной: %d символов", len(text))
    
    # 1. Проверка входных данных
    if not text or not text.strip():
        logger.error("Получен пустой текст")
        raise ValueError("Текст не может быть пустым")

    # 2. Очистка и подготовка текста
    try:
        cleaned_text = ' '.join(text.strip().split())
        cleaned_text = cleaned_text.replace('\u200b', '').replace('\ufeff', '')
        
        if len(cleaned_text) < 50:
            logger.warning("Текст слишком короткий: %d символов", len(cleaned_text))
            raise ValueError("Текст слишком короткий для анализа")
            
        MAX_LENGTH = 12000
        if len(cleaned_text) > MAX_LENGTH:
            logger.warning("Текст обрезан: %d > %d символов", len(cleaned_text), MAX_LENGTH)
            cleaned_text = cleaned_text[:MAX_LENGTH] + " [TRUNCATED]"
            
    except Exception as e:
        logger.error("Ошибка обработки текста: %s", str(e))
        raise ValueError(f"Ошибка обработки текста: {str(e)}")

    # 3. Формирование запроса
    full_prompt = PROMPT_TEMPLATE + " {" + cleaned_text + "}"
    
    payload = {
        "model": LOCAL_MODEL_NAME,
        "prompt": full_prompt,
        "stream": False,
        "options": {
            "temperature": 0.7,
            "top_p": 0.9,
            "max_tokens": 2000,
        }
    }

    logger.debug("Отправка запроса к OLLAMA. Длина промпта: %d", len(full_prompt))

    # 4. Отправка запроса
    try:
        start_time = time.time()
        response = requests.post(
            url=OLLAMA_ENDPOINT,
            json=payload,
            timeout=180  # Увеличенный таймаут для локальной модели
        )
        
        logger.info("Время выполнения: %.2f сек", time.time() - start_time)
        response.raise_for_status()
        
        response_data = response.json()
        
        # 5. Обработка ответа
        if "response" in response_data:
            return response_data["response"]
        else:
            logger.error("Неожиданный формат ответа: %s", json.dumps(response_data, indent=2))
            raise Exception("Некорректный формат ответа от модели")
            
    except requests.exceptions.HTTPError as e:
        error_msg = f"HTTP {e.response.status_code}"
        try:
            error_data = e.response.json()
            error_msg += f" - {error_data.get('error', 'Unknown error')}"
        except:
            error_msg += f" - {e.response.text[:200]}"
        logger.error("Ошибка API: %s", error_msg)
        raise Exception(f"API Error: {error_msg}")
        
    except requests.exceptions.Timeout:
        logger.error("Таймаут запроса (180 сек)")
        raise Exception("Превышено время ожидания ответа от модели")
        
    except Exception as e:
        logger.error("Необработанная ошибка: %s", str(e), exc_info=True)
        raise Exception(f"Unexpected error: {str(e)}")

def handle_api_response(response):
    """Обработка ответа от API"""
    try:
        data = response.json()
        logger.debug(f"Ответ API: {json.dumps(data, ensure_ascii=False)[:500]}...")
        
        if 'choices' not in data or len(data['choices']) == 0:
            raise ValueError("Некорректная структура ответа API")
            
        return data['choices'][0]['message']['content']
    except json.JSONDecodeError:
        logger.error(f"Невалидный JSON: {response.text[:500]}")
        raise

def parse_ai_response(response: str) -> dict:
    """Улучшенный парсинг ответа с регулярными выражениями"""
    try:
        result_match = re.search(r'result\s*=\s*([^;]+)', response, re.IGNORECASE)
        base_match = re.search(r'base\s*=\s*([^;]+)', response, re.IGNORECASE)

        if not result_match or not base_match:
            raise ValueError("Не найден обязательный параметр в ответе")

        return {
            "result": result_match.group(1).strip(),
            "base": base_match.group(1).strip(),
            "raw_response": response
        }
    except Exception as e:
        logger.error(f"Ошибка парсинга: {str(e)}")
        logger.debug(f"Полный ответ для отладки:\n{response}")
        raise

def main():
    try:
        OUTPUT_DIR.mkdir(exist_ok=True, parents=True)
    except Exception as e:
        logger.critical(f"Ошибка создания директории: {str(e)}")
        return

    files = list(INPUT_DIR.glob("output_part_*.json"))
    logger.info(f"Найдено файлов для обработки: {len(files)}")

    for idx, file_path in enumerate(files, 1):
        logger.info(f"Обработка файла {idx}/{len(files)}: {file_path.name}")
        
        try:
            process_file(file_path)
            
            # Случайная пауза между файлами
            if idx < len(files):
                delay = random.randint(20, 45)
                logger.info(f"Пауза {delay} сек. перед следующим файлом")
                time.sleep(delay)
                
        except Exception as e:
            logger.error(f"Фатальная ошибка обработки файла: {str(e)}")
            continue

if __name__ == "__main__":
    main()