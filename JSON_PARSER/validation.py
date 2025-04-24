import json
import re
from datetime import datetime
from pathlib import Path
from typing import Tuple, List, Dict
import logging
from json_validator import JsonValidator
from error_types import ErrorLevel, ValidationError

# Настройка логгера
logging.basicConfig(
    filename='D:/AI/vector/output_dir/check.log',
    encoding='utf-8',
    level=logging.INFO,
    format='%(asctime)s - %(levelname)s - %(message)s'
)
logger = logging.getLogger('json_validator')

class ValidationRules:
    TOP_LEVEL = [
        {
            'field': 'title',
            'required': True,
            'type': str,
            'min_length': 10
        },
        {
            'field': 'Postanovlenie suda apelyacionnoi instancii',
            'required': True,
            'type': str,
            'min_length': 900
        },
        {
            'field': 'stock',
            'required': True,
            'type': list,
            'min_items': 8,
            'item_rules': {
                'pattern': {
                    'Уникальный идентификатор дела': r'.{5,}',
                    'Номер дела': r'^.*[-/].*$',
                    'Осужденный.*': r'.+\(\Ст\.\s\d+\.\d+.*\)',
                    'Дата поступления': r'\d{2}\.\d{2}\.\d{4}',
                    'Номер судебного состава': r'.+',
                    'Номер дела в суде нижестоящей инстанции': r'/',
                    'Суд первой инстанции': r'\(.*\)',
                    'Текущее состояние': r'.+'
                }
            }
        },
        {
            'field': 'seaz',
            'required': True,
            'type': list,
            'min_items': 6,
            'item_rules': {
                'required_fields': ['Дата', 'Состояние'],
                'date_fields': ['Дата']
            }
        },
        {
            'field': 'docs',
            'required': True,
            'type': list,
            'min_items': 1,
            'item_type': str
        }
    ]

def validate_json_file(file_path: Path) -> Tuple[bool, List[ValidationError]]:
    validator = JsonValidator()
    errors = []
    has_errors = False
    
    try:
        with open(file_path, 'r', encoding='utf-8') as f:
            json_str = f.read()
            validator.build_line_map(json_str)

        data = json.loads(json_str)
        items_positions = []
        decoder = json.JSONDecoder()
        pos = 0
        while pos < len(json_str):
            if json_str[pos] in ' \t\n\r':
                pos += 1
            else:
                obj, pos = decoder.raw_decode(json_str, pos)
                items_positions.append(pos)

        for idx, (item, end_pos) in enumerate(zip(data, items_positions), 1):
            line_num = validator.get_line_number(end_pos)
            
            if not isinstance(item, dict):
                errors.append(ValidationError(
                    error_level=ErrorLevel.ERROR,
                    file=str(file_path),
                    element=idx,
                    line=line_num,
                    message="Элемент должен быть объектом"
                ))
                has_errors = True
                continue

            # Проверка полей верхнего уровня
            for rule in ValidationRules.TOP_LEVEL:
                field = rule['field']
                required = rule.get('required', False)
                
                if required and field not in item:
                    errors.append(make_error(f"Отсутствует обязательное поле '{field}'", ErrorLevel.ERROR))
                    has_errors = True
                    continue
                
                if field in item:
                    value = item[field]
                    
                    # Обработка null-значений
                    if value is None:
                        handle_null_value(field, rule, item, errors, idx, line_num, file_path)
                        value = item[field]
                    
                    # Проверка типа
                    if not isinstance(value, rule['type']):
                        errors.append(make_error(
                            f"Поле '{field}' должно быть {rule['type'].__name__}",
                            ErrorLevel.ERROR
                        ))
                        has_errors = True
                        continue
                    
                    # Специфические проверки для разных типов
                    if isinstance(value, str):
                        check_string_rules(field, value, rule, errors, line_num, file_path, idx)
                    
                    elif isinstance(value, list):
                        check_list_rules(field, value, rule, errors, line_num, file_path, idx, item)
                        has_errors = check_nested_rules(field, value, rule, errors, line_num, file_path, idx)

        # Логирование ошибок
        for error in errors:
            logger.error(f"{error.file} | Line: {error.line} | Element: {error.element} | {error.message}")

        return not has_errors, errors

    except json.JSONDecodeError as e:
        line = validator.get_line_number(e.pos)
        error = make_error(f"Синтаксическая ошибка JSON: {e.msg}", ErrorLevel.ERROR, line=line)
        logger.error(str(error))
        return False, [error]
    except Exception as e:
        error = make_error(f"Непредвиденная ошибка: {str(e)}", ErrorLevel.ERROR)
        logger.error(str(error))
        return False, [error]

# Вспомогательные функции
def make_error(message: str, level: ErrorLevel, **kwargs) -> ValidationError:
    return ValidationError(
        error_level=level,
        message=message,
        **kwargs
    )

def handle_null_value(field: str, rule: dict, item: dict, errors: list, idx: int, line_num: int, file_path: str):
    default_value = "" if rule['type'] == str else []
    item[field] = default_value
    errors.append(make_error(
        f"Поле '{field}' заменено на {default_value}",
        ErrorLevel.NOTICE,
        file=str(file_path),
        element=idx,
        line=line_num
    ))

def check_string_rules(field: str, value: str, rule: dict, errors: list, line_num: int, file_path: str, idx: int):
    if 'min_length' in rule and len(value) < rule['min_length']:
        errors.append(make_error(
            f"Поле '{field}' слишком короткое (мин. {rule['min_length']} симв.)",
            ErrorLevel.NOTICE,
            file=str(file_path),
            element=idx,
            line=line_num
        ))

def check_list_rules(field: str, value: list, rule: dict, errors: list, line_num: int, file_path: str, idx: int, item: dict):
    if 'min_items' in rule and len(value) < rule['min_items']:
        errors.append(make_error(
            f"Поле '{field}' содержит мало элементов (мин. {rule['min_items']})",
            ErrorLevel.WARNING,
            file=str(file_path),
            element=idx,
            line=line_num
        ))
    
    if 'item_type' in rule:
        for i, element in enumerate(value):
            if not isinstance(element, rule['item_type']):
                errors.append(make_error(
                    f"Элемент {i+1} в поле '{field}' должен быть {rule['item_type'].__name__}",
                    ErrorLevel.ERROR,
                    file=str(file_path),
                    element=idx,
                    line=line_num
                ))

def check_nested_rules(field: str, items: list, rule: dict, errors: list, line_num: int, file_path: str, idx: int) -> bool:
    has_errors = False
    if field == 'stock' and 'item_rules' in rule:
        for i, item in enumerate(items):
            if not isinstance(item, str):
                continue
                
            for pattern_name, regex in rule['item_rules']['pattern'].items():
                if re.match(pattern_name, item):
                    if not re.search(regex, item.split(": ")[-1]):
                        errors.append(make_error(
                            f"stock[{i}] - '{pattern_name}': несоответствие формата",
                            ErrorLevel.ERROR,
                            file=str(file_path),
                            element=idx,
                            line=line_num
                        ))
                        has_errors = True
    
    elif field == 'seaz' and 'item_rules' in rule:
        for i, item in enumerate(items):
            if isinstance(item, str) and ':' in item:
                key, value = item.split(": ", 1)
                if key in rule['item_rules']['required_fields'] and not value.strip():
                    errors.append(make_error(
                        f"seaz[{i}] - '{key}': обязательное поле не может быть пустым",
                        ErrorLevel.ERROR,
                        file=str(file_path),
                        element=idx,
                        line=line_num
                    ))
                    has_errors = True
                
                if key in rule['item_rules']['date_fields']:
                    try:
                        datetime.strptime(value.strip(), "%d.%m.%Y")
                    except ValueError:
                        errors.append(make_error(
                            f"seaz[{i}] - '{key}': неверный формат даты",
                            ErrorLevel.ERROR,
                            file=str(file_path),
                            element=idx,
                            line=line_num
                        ))
                        has_errors = True
    
    return has_errors