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
        self.config_file = self.configs_dir / "config.py"
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
            self.configs_dir
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

SQL_CONFIG = {
    'host': 'localhost',
    'user': 'admin',
    'password': 'secret',
    'database': 'mydb'
}

OPENROUTER_API_KEY = 'your-api-key-here'
VECTOR_DB_PATH = '/path/to/vector/db'
"""
        self.config_file.write_text(default_content, encoding='utf-8')

    def _check_preconditions(self):
        return self.state.state == "uninitialized"