import psycopg2
import json
from datetime import datetime
import os

# Данные для подключения к базе данных
db_params = {
    "host": "pg-363d33c1-wcastleswc-a8d6.c.aivencloud.com",
    "port": 21026,
    "database": "defaultdb",
    "user": "avnadmin",
    "password": "AVNS_gp7bgTT2_vsasrmqr5V",
    "sslmode": "require"
}

def load_cases_data(file_path):
    """Загрузка данных из JSON-файла"""
    try:
        with open(file_path, 'r', encoding='utf-8') as file:
            return json.load(file)
    except FileNotFoundError:
        print(f"Ошибка: Файл {file_path} не найден!")
        exit(1)
    except json.JSONDecodeError:
        print(f"Ошибка: Некорректный формат JSON в файле {file_path}!")
        exit(1)

def parse_stock(stock):
    """Парсинг массива stock в структурированные данные"""
    data = {}
    
    try:
        #Уникальный идентификатор дела
        text_found = any('икальный идентификатор де' in line for line in stock)
        colon_found = False
        value_valid = False

        if text_found:
            target_line = next((line for line in stock if 'икальный идентификатор де' in line), '')
            colon_found = ':' in target_line

            if colon_found:
                parts = target_line.split(':', 1)
                if len(parts) > 1 and parts[1].strip().strip('"'):
                    value_valid = True

        if text_found and colon_found and value_valid:
            data['stock_id'] = target_line.split(': ', 1)[1].strip()
        else:
            print("error [Уникальный идентификатор дела] [stock_id] text_found  colon_found  value_valid "  +  str(text_found) + " " + str(colon_found) +" "+ str(value_valid))


        # Номер дела
        text_found = any('омер дел' in line for line in stock)
        colon_found = False
        value_valid = False

        if text_found:
            target_line = next((line for line in stock if 'омер дел' in line), '')
            colon_found = ':' in target_line

            if colon_found:
                parts = target_line.split(':', 1)
                if len(parts) > 1 and parts[1].strip().strip('"'):
                    value_valid = True

        if text_found and colon_found and value_valid:
            data['stock_number'] = target_line.split(': ', 1)[1].strip()
        else:
            print("error [Номер дела] [stock_number] invalid in " + data['stock_id'] + " Flags: text_found  colon_found  value_valid "  +  str(text_found) + " " + str(colon_found) +" "+ str(value_valid))


        any_person_found = False
        text_found = any('сужденный (оправданный, обвиняем' in line for line in stock)
        colon_found = False
        value_valid = False

        if text_found:
            target_line = next((line for line in stock if 'жденный (оправданный, обвиня' in line), '')
            colon_found = ':' in target_line

            if colon_found:
                parts = target_line.split(':', 1)
                if len(parts) > 1 and parts[1].strip().strip('"'):
                    value_valid = True

        if text_found and colon_found and value_valid:
            person_info = target_line.split(': ', 1)[1].strip()
            any_person_found = True
            if '\n' in person_info:
                name_part, reason_part = person_info.split('\n', 1)
                data['stock_person_name'] = name_part.strip()
                reason = reason_part.replace('(', '').replace(')', '').strip()
                data['stock_person_reason'] = reason if reason else None
            else:
                data['stock_person_name'] = person_info
                data['stock_person_reason'] = None            


        text_found = any('одсудимы' in line for line in stock)
        colon_found = False
        value_valid = False

        if text_found:
            target_line = next((line for line in stock if 'одсудимы' in line), '')
            colon_found = ':' in target_line

            if colon_found:
                parts = target_line.split(':', 1)
                if len(parts) > 1 and parts[1].strip().strip('"'):
                    value_valid = True

        if text_found and colon_found and value_valid:
            person_info = target_line.split(': ', 1)[1].strip()
            any_person_found = True            
            if '\n' in person_info:
                name_part, reason_part = person_info.split('\n', 1)
                data['stock_person_name'] = name_part.strip()
                reason = reason_part.replace('(', '').replace(')', '').strip()
                data['stock_person_reason'] = reason if reason else None
            else:
                data['stock_person_name'] = person_info
                data['stock_person_reason'] = None            


        text_found = any('аявител' in line for line in stock)
        colon_found = False
        value_valid = False

        if text_found:
            target_line = next((line for line in stock if 'аявител' in line), '')
            colon_found = ':' in target_line

            if colon_found:
                parts = target_line.split(':', 1)
                if len(parts) > 1 and parts[1].strip().strip('"'):
                    value_valid = True

        if text_found and colon_found and value_valid:
            person_info = target_line.split(': ', 1)[1].strip()
            any_person_found = True            
            if '\n' in person_info:
                name_part, reason_part = person_info.split('\n', 1)
                data['stock_person_name'] = name_part.strip()
                reason = reason_part.replace('(', '').replace(')', '').strip()
                data['stock_person_reason'] = reason if reason else None
            else:
                data['stock_person_name'] = person_info
                data['stock_person_reason'] = None            


        if not any_person_found:
            print("error [Заявитель] [person_info] invalid in " + data['stock_id'] + " Flags: text_found  colon_found  value_valid "  +  str(text_found) + " " + str(colon_found) +" "+ str(value_valid))



        text_found = any('ата поступления дела в апелляционную инстанц' in line for line in stock)
        colon_found = False
        value_valid = False

        if text_found:
            target_line = next((line for line in stock if 'ата поступления дела в апелляционную инстанц' in line), '')
            colon_found = ':' in target_line

            if colon_found:
                parts = target_line.split(':', 1)
                if len(parts) > 1 and parts[1].strip().strip('"'):
                    value_valid = True

        if text_found and colon_found and value_valid:
            data['stock_date_in'] = target_line.split(': ', 1)[1].strip()
            if '.' in data['stock_date_in']:
                data['stock_date_in'] = datetime.strptime(data['stock_date_in'], '%d.%m.%Y').date()
            else:
                data['stock_date_in'] = None
                print("error  stock_date_in  colon_found  incorrect format"  +  str(text_found) + " " + str(colon_found) +" "+ str(value_valid))
        else:
            print("error [Дата поступления дела в апелляционную инстанцию] [stock_date_in]  colon_found  value_valid " +  str(text_found) + " " + str(colon_found) +" "+ str(value_valid))


        text_found = any('омер судебного состав' in line for line in stock)
        colon_found = False
        value_valid = False

        if text_found:
            target_line = next((line for line in stock if 'омер судебного состав' in line), '')
            colon_found = ':' in target_line

            if colon_found:
                parts = target_line.split(':', 1)
                if len(parts) > 1 and parts[1].strip().strip('"'):
                    value_valid = True

        if text_found and colon_found and value_valid:
            data['stock_order_number'] = target_line.split(': ', 1)[1].strip()
        else:
            print("error [Номер судебного состава] [stock_order_number] invalid in " + data['stock_id'] + " Flags: text_found  colon_found  value_valid "  +  str(text_found) + " " + str(colon_found) +" "+ str(value_valid))

        text_found = any('омер дела в суде нижестоящей инстанци' in line for line in stock)
        colon_found = False
        value_valid = False

        if text_found:
            target_line = next((line for line in stock if 'омер дела в суде нижестоящей инстанци' in line), '')
            colon_found = ':' in target_line

            if colon_found:
                parts = target_line.split(':', 1)
                if len(parts) > 1 and parts[1].strip().strip('"'):
                    value_valid = True

        if text_found and colon_found and value_valid:
            data['stock_lower_instance_case_id'] = target_line.split(': ', 1)[1].strip()
        else:
            print("error [Номер дела в суде нижестоящей инстанции] [stock_lower_instance_case_id] invalid in " + data['stock_id'] + " Flags: text_found  colon_found  value_valid " +  str(text_found) + " " + str(colon_found) +" "+ str(value_valid))


        any_jury_judge_found = False
        text_found = any('уд первой инстанции, судь' in line for line in stock)
        colon_found = False
        value_valid = False

        if text_found:
            target_line = next((line for line in stock if 'уд первой инстанции, судь' in line), '')
            colon_found = ':' in target_line

            if colon_found:
                parts = target_line.split(':', 1)
                if len(parts) > 1 and parts[1].strip().strip('"'):
                    value_valid = True


        if text_found and colon_found and value_valid:
            person_info = target_line.split(': ', 1)[1].strip()
            any_jury_judge_found = True
            if '(' in person_info:
                name_part, reason_part = person_info.split('(', 1)
                data['stock_lower_jury'] = name_part.strip()
                judge = reason_part.replace('(', '').replace(')', '').strip()
                data['stock_lower_jury_judge'] = judge if judge else None
            else:
                data['stock_lower_jury'] = person_info
                data['stock_lower_jury_judge'] = None            


        text_found = any('Cудь' in line for line in stock)
        colon_found = False
        value_valid = False

        if text_found:
            target_line = next((line for line in stock if 'Cудь' in line), '')
            colon_found = ':' in target_line

            if colon_found:
                parts = target_line.split(':', 1)
                if len(parts) > 1 and parts[1].strip().strip('"'):
                    value_valid = True

        if text_found and colon_found and value_valid:
            data['stock_lower_jury_judge'] = target_line.split(': ', 1)[1].strip()
            any_jury_judge_found = True
            if '.' in data['stock_lower_jury_judge']:
                data['stock_lower_jury_judge'] =data['stock_lower_jury_judge']
            else:
                data['stock_lower_jury_judge'] = None
                print("error  stock_lower_jury_judge  colon_found  incorrect format "  +  str(text_found) + " " + str(colon_found) +" "+ str(value_valid))
        
        if not any_jury_judge_found:
             print("error [Cудья] [stock_lower_jury] [stock_lower_jury_judge] invalid in " + data['stock_id'] + " Flags: text_found  colon_found  value_valid "  +  str(text_found) + " " + str(colon_found) +" "+ str(value_valid))



            

        text_found = any('екущее состояни' in line for line in stock)
        colon_found = False
        value_valid = False

        if text_found:
            target_line = next((line for line in stock if 'екущее состояни' in line), '')
            colon_found = ':' in target_line

            if colon_found:
                parts = target_line.split(':', 1)
                if len(parts) > 1 and parts[1].strip().strip('"'):
                    value_valid = True

        if text_found and colon_found and value_valid:
            data['stock_current_state'] = target_line.split(': ', 1)[1].strip()
        else:
            print("error [Текущее состояние] [stock_current_state] invalid in " + data['stock_id'] + " Flags: text_found  colon_found  value_valid " +  str(text_found) + " " + str(colon_found) +" "+ str(value_valid))

    except (IndexError, KeyError, ValueError, AttributeError) as e:
        print(f"Ошибка парсинга stock: {e}")
    return data

def parse_seaz(seaz):
    """Парсинг массива seaz в структурированные данные"""
    data = {}
    try:
        data['seaz_date'] = datetime.strptime(seaz[0].split(': ', 1)[1].strip(), '%d.%m.%Y').date()
        data['seaz_state'] = seaz[1].split(': ', 1)[1].strip()
        data['seaz_doc'] = seaz[2].split(': ', 1)[1].strip()
        data['seaz_place'] = seaz[4].split(': ', 1)[1].strip()
        data['seaz_comment'] = seaz[5].split(': ', 1)[1].strip()
    except (IndexError, KeyError, ValueError) as e:
        print(f"Ошибка парсинга seaz: {e}")
    return data

def main():
    # Загрузка данных из файла
    file_path = r'C:\PyProjects\Consul 25452.json'
    cases_data = load_cases_data(file_path)

    try:
        # Подключаемся к базе данных
        conn = psycopg2.connect(**db_params)
        cur = conn.cursor()
        
        # Удаляем существующую таблицу если есть
        cur.execute("DROP TABLE IF EXISTS cases;")
        
        # Создаем новую таблицу с комментариями
        create_table_query = """
        CREATE TABLE cases (
            id SERIAL PRIMARY KEY,
            title TEXT,
            postanovlenie TEXT,
            stock_id TEXT,
            stock_number TEXT,
            stock_person_name TEXT,
            stock_person_reason TEXT,
            stock_date_in DATE,
            stock_order_number TEXT,
            stock_lower_instance_case_id TEXT,
            stock_lower_jury TEXT,
            stock_lower_jury_judge TEXT,
            stock_current_state TEXT,
            seaz_date DATE,
            seaz_state TEXT,
            seaz_doc TEXT,
            seaz_place TEXT,
            seaz_comment TEXT,
            docs TEXT
        );
        
        COMMENT ON TABLE cases IS 'Таблица судебных дел';
        COMMENT ON COLUMN cases.title IS 'Название дела';
        COMMENT ON COLUMN cases.postanovlenie IS 'Текст постановления';
        COMMENT ON COLUMN cases.stock_id IS 'Уникальный идентификатор дела';
        COMMENT ON COLUMN cases.stock_number IS 'Номер дела';
        COMMENT ON COLUMN cases.stock_person_name IS 'Осужденный/обвиняемый';
        COMMENT ON COLUMN cases.stock_person_reason IS 'Статья';
        COMMENT ON COLUMN cases.stock_date_in IS 'Дата поступления в апелляцию';
        COMMENT ON COLUMN cases.stock_order_number IS 'Номер судебного состава';
        COMMENT ON COLUMN cases.stock_lower_instance_case_id IS 'Номер дела в нижестоящем суде';
        COMMENT ON COLUMN cases.stock_lower_jury IS 'Суд первой инстанции';
        COMMENT ON COLUMN cases.stock_lower_jury_judge IS 'Судья первой инстанции';
        COMMENT ON COLUMN cases.stock_current_state IS 'Текущий статус дела';
        COMMENT ON COLUMN cases.seaz_date IS 'Дата состояния';
        COMMENT ON COLUMN cases.seaz_state IS 'Статус регистрации';
        COMMENT ON COLUMN cases.seaz_doc IS 'Основание документа';
        COMMENT ON COLUMN cases.seaz_place IS 'Местонахождение дела';
        COMMENT ON COLUMN cases.seaz_comment IS 'Комментарий к состоянию';
        COMMENT ON COLUMN cases.docs IS 'Список документов';
        """
        cur.execute(create_table_query)
        
        # Вставляем данные из JSON
        for case in cases_data:
            # Парсинг данных
            stock_data = parse_stock(case['stock'])
            seaz_data = parse_seaz(case['seaz'])
            docs_str = '\n'.join(case['docs'])
            
            # Вставка в БД
            cur.execute(
                """INSERT INTO cases (
                    title, postanovlenie, 
                    stock_id, stock_number, 
                    stock_person_name, stock_person_reason,
                    stock_date_in, stock_order_number, 
                    stock_lower_instance_case_id,
                    stock_lower_jury, stock_lower_jury_judge,
                    stock_current_state,
                    seaz_date, seaz_state, seaz_doc, seaz_place, seaz_comment,
                    docs
                ) VALUES (
                    %s, %s, %s, %s, %s, %s, %s, %s, %s, %s, %s, %s, %s, %s, %s, %s, %s, %s
                )""",
                (
                    case['title'],
                    case['Postanovlenie suda apelyacionnoi instancii'],
                    stock_data.get('stock_id'),
                    stock_data.get('stock_number'),
                    stock_data.get('stock_person_name'),  # новое поле
                    stock_data.get('stock_person_reason'), # новое поле
                    stock_data.get('stock_date_in'),
                    stock_data.get('stock_order_number'),
                    stock_data.get('stock_lower_instance_case_id'),
                    stock_data.get('stock_lower_jury'),    # новое поле
                    stock_data.get('stock_lower_jury_judge'), # новое поле
                    stock_data.get('stock_current_state'),
                    seaz_data.get('seaz_date'),
                    seaz_data.get('seaz_state'),
                    seaz_data.get('seaz_doc'),
                    seaz_data.get('seaz_place'),
                    seaz_data.get('seaz_comment'),
                    docs_str
                )
            )
        
        # Фиксируем изменения и закрываем соединение
        conn.commit()
        print(f"Успешно обработано {len(cases_data)} записей!")
        
    except Exception as e:
        print(f"Ошибка: {e}")
        conn.rollback()
    finally:
        if conn:
            cur.close()
            conn.close()

if __name__ == "__main__":
    main()