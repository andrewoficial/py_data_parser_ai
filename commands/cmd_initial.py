import argparse
from pathlib import Path
from commands.base import BaseCommand
from core.system_state import SystemState

class InitialCommand(BaseCommand):
    def __init__(self, project_root: Path):
        self.project_root = project_root
        self.default_output_dir = project_root / "dir_output"
        self.default_output_dir_tmp = project_root / "dir_temp"
        self.default_output_dir_logs = project_root / "dir_logs"
        self.configs_dir = project_root / "configs"
        self.config_file = self.configs_dir / "conf.py"

        self.sources_dir = self.project_root / "dir_input" # Файлы для анализа
        self.json_dir = self.project_root / "dir_temp/inp_json" # Найденные json
        self.cpp_dir = self.project_root / "dir_temp/inp_cpp" # Найденные cpp
        self.qaa_dir = self.project_root / "dir_temp/inp_q_and_a" # Найденные q  and a файлы
        self.json_split_dir = self.project_root / "dir_temp/inp_json_split" # Найденные json после разделения
        self.cache_dir = self.project_root / "dir_temp/file_hashes" # Хранение хешей
        self.cache_path = self.project_root / "dir_temp/file_hashes/file_hashes.json" # Хранение файл
        self.logs_dir = self.project_root / "dir_logs" # Хранение хешей
        self.vector_store_dir = self.project_root / "dir_output/summarization" # Хранение обощений  
        self.unsort_dir = self.project_root / "dir_temp/inp_unsort" # Не были сортированы      
        self.json_syntax_ok = self.project_root / "dir_temp/inp_json_syntax_ok" # Json после успешной проверки синтаксиса
        self.json_syntax_err = self.project_root / "dir_temp/inp_json_syntax_err" # Json после провала проверки синтаксиса
        self.state = SystemState()

    @property
    def name(self) -> str:
        return "initial"

    @property
    def description(self) -> str:
        return "Initialize project directories and configuration"

    def setup_parser(self, parser: argparse.ArgumentParser):
        parser.add_argument(
            '--dry-run',
            action='store_true',
            help='Simulate creation without actual changes'
        )

    def execute(self, args):
        print("Инициализация проекта...")
        self.state.set_state("uninitialized")  # Принудительно ставим состояние

        # Список всех папок, которые нужно создать при инициализации
        required_dirs = [
            self.default_output_dir,
            self.default_output_dir_tmp,
            self.default_output_dir_logs,
            self.configs_dir,
            self.sources_dir,
            self.json_dir,
            self.cpp_dir,
            self.qaa_dir,
            self.cache_dir,
            self.logs_dir,
            self.unsort_dir,
            self.json_syntax_err,
            self.json_syntax_ok,
            self.vector_store_dir
        ]

        for dir_path in required_dirs:
            if not dir_path.exists():
                if args.dry_run:
                    print(f"[DRY] Would create directory: {dir_path}")
                else:
                    dir_path.mkdir(parents=True, exist_ok=True)
                    print(f"[OK] Created directory: {dir_path}")
            else:
                print(f"[SKIP] Directory already exists: {dir_path}")

        # Проверка и создание конфигурационного файла
        if not self.config_file.exists():
            if args.dry_run:
                print(f"[DRY] Would create config file: {self.config_file}")
            else:
                self._create_default_config()
                print(f"[OK] Created default config: {self.config_file}")
        else:
            print(f"[SKIP] Config file already exists: {self.config_file}")

    def _create_default_config(self):
        default_content = """# Auto-generated config.py

from pydantic_settings import BaseSettings
from pathlib import Path
from typing import Set


class PathConfig(BaseSettings):
    base_dir: Path = Path(__file__).resolve().parent.parent

    sources_dir: Path | None = None
    json_dir: Path | None = None
    json_split_dir: Path | None = None
    vector_store_dir: Path | None = None
    cache_path: Path | None = None

    model_config = {
        "extra": "allow",
        "env_prefix": "path_",
        "env_file": ".env"
    }

    def __init__(self, **kwargs):
        super().__init__(**kwargs)

        self.sources_dir = self.base_dir / "dir_input" # Файлы для анализа
        self.json_dir = self.sources_dir / "dir_temp/inp_json" # Найденные json
        self.cpp_dir = self.sources_dir / "dir_temp/inp_cpp" # Найденные cpp
        self.qaa_dir = self.sources_dir / "dir_temp/inp_q_and_a" # Найденные q  and a файлы
        self.unsort_dir = self.sources_dir / "dir_temp/inp_unsort" # Не были сортированы
        self.json_split_dir = self.sources_dir / "dir_temp/inp_json_split" # Найденные json после разделения
        self.cache_dir = self.base_dir / "dir_temp/file_hashes" # Хранение хешей
        self.cache_path = self.base_dir / "dir_temp/file_hashes/file_hashes.json" # Хранение файл
        self.logs_dir = self.base_dir / "dir_logs" # Хранение хешей
        self.vector_store_dir = self.base_dir / "dir_output/summarization" # Хранение обощений
        self.ignore_dirs = self.base_dir / "dir_input/tmp" # Хранение обощений
        self.json_syntax_ok = self.base_dir / "dir_temp/inp_json_syntax_ok" # Json после успешной проверки синтаксиса
        self.json_syntax_err = self.base_dir / "dir_temp/inp_json_syntax_err" # Json после провала проверки синтаксиса


class VectorConfig(BaseSettings):
    chunk_size: int = 256
    overlap: int = 32
    ignore_dirs: Set[str] = {"build", "test", "thirdparty"}
    file_extensions_cpp: Set[str] = {"cpp", "hpp", "h", "cc", "cxx"}
    file_extensions_json: Set[str] = {"json"}
    max_file_size: int = 2 * 1024 * 1024
    supported_extensions: Set[str] = file_extensions_cpp.union(file_extensions_json)
    model_config = {
        "extra": "allow",
        "env_prefix": "vector_",
        "env_file": ".env"
    }        


class DBConfig(BaseSettings):
    host: str
    port: int
    name: str
    user: str
    password: str
    sslmode: str = "require"
    model_config = {
        "extra": "allow",
        "env_prefix": "db_",
        "env_file": ".env"
    } 


class ModelConfig(BaseSettings):
    openrouter_api_key: str
    model: str = "meta-llama/llama-3.1-8b-instruct:free"
    ollama_endpoint: str = "http://127.0.0.1:11434/api/generate"
    local_model_name: str = "llama3.2"
    prompt_template: str = (
        "Составь на основе юридического документа JSON с полями 'result' (обвинительный, оправдательный удовлетворено ли ходатайство) и "
        "'base' (статьи на основании которых вынесено решение с пунктами через запятую). Исходные данные: "
    )

    model_config = {
        "extra": "allow",
        "env_prefix": "model_",
        "env_file": ".env"
    } 
"""
        self.config_file.write_text(default_content, encoding='utf-8')

    def _check_preconditions(self):
        return self.state.state == "uninitialized"