import argparse
from pathlib import Path
from commands.base import BaseCommand
from core.system_state import SystemState


class ConfigureCommand(BaseCommand):
    def __init__(self, project_root: Path):
        self.project_root = project_root
        self.configs_dir = self.project_root / "configs"
        self.config_file = self.configs_dir / "conf.py"
        self.env_file = self.project_root / ".env"
        self.state = SystemState()

    @property
    def name(self) -> str:
        return "configure"

    @property
    def description(self) -> str:
        return "Create default .env and configs/conf.py if missing"

    def setup_parser(self, parser: argparse.ArgumentParser):
        parser.add_argument("--dry-run", action="store_true", help="Preview what would be created")

    def execute(self, args):
        print("Настройка конфигурации проекта...")
        self.state.set_state("uninitialized")  # Начальное состояние
        if not self.configs_dir.exists():
            if args.dry_run:
                print(f"[DRY] Would create configs directory: {self.configs_dir}")
            else:
                self.configs_dir.mkdir(parents=True, exist_ok=True)
                print(f"[OK] Created configs directory: {self.configs_dir}")

        self._create_env_file(args)
        self._create_config_file(args)

    def _create_env_file(self, args):
        if not self.env_file.exists():
            if args.dry_run:
                print(f"[DRY] Would create .env file at {self.env_file}")
            else:
                env_template = self._get_env_template()
                self.env_file.write_text(env_template, encoding="utf-8")
                print(f"[OK] Created .env file: {self.env_file}")
        else:
            print(f"[SKIP] .env file already exists: {self.env_file}")
        
        if not args.dry_run:
            from core.settings import SettingsLoader
            SettingsLoader.load_all(self.project_root)
            self.state.set_state("configured")
            print("[OK] Состояние системы: configured")

    def _create_config_file(self, args):
        if not self.config_file.exists():
            if args.dry_run:
                print(f"[DRY] Would create config file at {self.config_file}")
            else:
                config_template = self._get_config_template()
                self.config_file.write_text(config_template, encoding="utf-8")
                print(f"[OK] Created config file: {self.config_file}")
        else:
            print(f"[SKIP] Config file already exists: {self.config_file}")

    def _get_env_template(self) -> str:
        return """# Auto-generated .env

# DB
DB_HOST=localhost
DB_PORT=5432
DB_NAME=mydb
DB_USER=admin
DB_PASSWORD=secret

# API
OPENROUTER_API_KEY=your-api-key-here

# Model
MODEL_NAME=meta-llama/llama-3.1-8b-instruct:free
LOCAL_MODEL_NAME=llama3.2
OLLAMA_ENDPOINT=http://127.0.0.1:11434/api/generate
"""

    def _get_config_template(self) -> str:
        return '''from pydantic import BaseSettings
from pathlib import Path
from typing import Set


class PathConfig(BaseSettings):
    base_dir: Path = Path(__file__).resolve().parent.parent

    sources_dir: Path = None
    json_dir: Path = None
    json_split_dir: Path = None
    vector_store_dir: Path = None
    cache_path: Path = None

    class Config:
        env_prefix = "path_"
        env_file = ".env"

    def __init__(self, **kwargs):
        super().__init__(**kwargs)
        self.sources_dir = self.base_dir / "sources"
        self.json_dir = self.sources_dir
        self.json_split_dir = self.sources_dir
        self.vector_store_dir = self.base_dir / "vector_store"
        self.cache_path = self.base_dir / "file_hashes.json"


class VectorConfig(BaseSettings):
    chunk_size: int = 256
    overlap: int = 32
    ignore_dirs: Set[str] = {"build", "test", "thirdparty"}
    file_extensions_cpp: Set[str] = {"cpp", "hpp", "h", "cc", "cxx"}
    file_extensions_json: Set[str] = {"json"}
    max_file_size: int = 2 * 1024 * 1024

    class Config:
        env_prefix = "vector_"
        env_file = ".env"


class DBConfig(BaseSettings):
    host: str
    port: int
    database: str
    user: str
    password: str
    sslmode: str = "require"

    class Config:
        env_prefix = "db_"
        env_file = ".env"


class ModelConfig(BaseSettings):
    openrouter_api_key: str
    input_dir: Path = Path("D:/AI/vector/output_dir")
    output_dir: Path = Path("D:/AI/vector/summarization")
    model: str = "meta-llama/llama-3.1-8b-instruct:free"
    ollama_endpoint: str = "http://127.0.0.1:11434/api/generate"
    local_model_name: str = "llama3.2"
    prompt_template: str = (
        "Составь на основе юридического документа JSON с полями 'result' (обвинительный, оправдательный удовлетворено ли ходатайство) и "
        "'base' (статьи на основании которых вынесено решение с пунктами через запятую). Исходные данные: "
    )

    class Config:
        env_prefix = "model_"
        env_file = ".env"
'''

    def _check_preconditions(self):
        return self.state.state == "uninitialized"
