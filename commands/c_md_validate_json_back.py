# commands/cmd_validate_json.py
import json
import re
import shutil
import logging
from datetime import datetime
from pathlib import Path
from typing import List, Tuple, Dict
from commands.base import BaseCommand
from core.settings import get_path_config
from core.json_validator import JsonValidator
from core.error_types import ErrorLevel, ValidationError

class ValidateJsonCommand(BaseCommand):
    def __init__(self, project_root: Path):
        self.path_config = get_path_config()
        self._setup_logger()
        self.validator = JsonValidator()
        self.validation_rules = self._load_validation_rules()

    @property
    def name(self) -> str:
        return "json_validate"
    
    @property
    def description(self) -> str:
        return "Validate JSON files structure and syntax"

    def setup_parser(self, parser):
        parser.add_argument(
            'command',
            nargs='?',
            help='Command to show help for'
        )

    def _setup_logger(self):
        self.logs_dir = self.path_config.logs_dir
        self.logs_dir.mkdir(exist_ok=True, parents=True)
        
        logging.basicConfig(
            filename=str(self.logs_dir / 'validation.log'),
            encoding='utf-8',
            level=logging.INFO,
            format='%(asctime)s - %(levelname)s - %(message)s'
        )
        self.logger = logging.getLogger('json_validator')

    def _load_validation_rules(self):
        return {
            'TOP_LEVEL': [
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
        }

    def execute(self, args):
        print("RUN JSON PARSER")
        json_dir = self.path_config.json_dir
        ok_dir = self.path_config.json_syntax_ok
        err_dir = self.path_config.json_syntax_err

        ok_dir.mkdir(exist_ok=True, parents=True)
        err_dir.mkdir(exist_ok=True, parents=True)

        stats = {'total': 0, 'valid': 0, 'invalid': 0}
        
        for json_file in json_dir.glob('**/*.json'):
            stats['total'] += 1
            is_valid, errors = self._validate_file(json_file)
            
            if is_valid:
                self._copy_file(json_file, ok_dir)
                stats['valid'] += 1
            else:
                self._copy_file(json_file, err_dir)
                stats['invalid'] += 1
                self._log_errors(errors)

        print("\nValidation results:")
        print(f"Total files: {stats['total']}")
        print(f"Valid files: {stats['valid']}")
        print(f"Invalid files: {stats['invalid']}")
        print(f"Log file: {self.logs_dir / 'validation.log'}")

    def _validate_file(self, file_path: Path) -> Tuple[bool, List[ValidationError]]:
        try:
            with open(file_path, 'r', encoding='utf-8') as f:
                json_str = f.read()
                self.validator.build_line_map(json_str)

            data = json.loads(json_str)
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
                return len(errors) == 0, errors

            return not has_errors, errors

        except json.JSONDecodeError as e:
            line = self.validator.get_line_number(e.pos)
            error = ValidationError(
                error_level=ErrorLevel.ERROR,
                file=str(file_path),
                element=0,
                line=line,
                message=f"JSON syntax error: {e.msg}"
            )
            return False, [error]
        except Exception as e:
            error = ValidationError(
                error_level=ErrorLevel.ERROR,
                file=str(file_path),
                message=f"Unexpected error: {str(e)}"
            )
            return False, [error]

    def _copy_file(self, src: Path, dest_dir: Path):
        try:
            dest = dest_dir / src.name
            shutil.copy2(src, dest)
            self.logger.info(f"Copied {src.name} to {dest_dir.name}")
        except Exception as e:
            self.logger.error(f"Failed to copy {src.name}: {str(e)}")

    def _log_errors(self, errors: List[ValidationError]):
        for error in errors:
            log_message = (
                f"{error.file} | Line: {error.line} | "
                f"Element: {error.element} | {error.message}"
            )
            self.logger.error(log_message)


             #Вспомогательные функции
    def _make_error(message: str, level: ErrorLevel, **kwargs) -> ValidationError:
        return ValidationError(
            error_level=level,
            message=message,
            **kwargs
        )

    def _handle_null_value(field: str, rule: dict, item: dict, errors: list, idx: int, line_num: int, file_path: str):
        default_value = "" if rule['type'] == str else []
        item[field] = default_value
        errors.append(make_error(
            f"Поле '{field}' заменено на {default_value}",
            ErrorLevel.NOTICE,
            file=str(file_path),
            element=idx,
            line=line_num
        ))

    def _check_string_rules(field: str, value: str, rule: dict, errors: list, line_num: int, file_path: str, idx: int):
        if 'min_length' in rule and len(value) < rule['min_length']:
            errors.append(make_error(
                f"Поле '{field}' слишком короткое (мин. {rule['min_length']} симв.)",
                ErrorLevel.NOTICE,
                file=str(file_path),
                element=idx,
                line=line_num
            ))

    def _check_list_rules(field: str, value: list, rule: dict, errors: list, line_num: int, file_path: str, idx: int, item: dict):
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

    def _check_nested_rules(field: str, items: list, rule: dict, errors: list, line_num: int, file_path: str, idx: int) -> bool:
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

    def _check_preconditions(self):
        return self.state.state == "uninitialized"    

                     

