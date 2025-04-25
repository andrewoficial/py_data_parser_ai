# commands/cmd_sort_files.py
import shutil
from pathlib import Path
from typing import Set
from commands.base import BaseCommand
from core.settings import get_path_config, get_vector_config

class SortFilesCommand(BaseCommand):
    def __init__(self, project_root: Path):

        self.path_config = get_path_config()
        self.vector_config = get_vector_config()
        
        # Инициализируем целевые директории
        self.base_dir = self.path_config.base_dir
        self.cpp_dir = self.path_config.cpp_dir
        #print(self.base_dir)
        #print(self.cpp_dir)
        self.json_dir = self.path_config.json_dir
        self.qaa_dir = self.path_config.qaa_dir
        
        # Создаем директории при инициализации
        self.cpp_dir.mkdir(exist_ok=True, parents=True)
        self.json_dir.mkdir(exist_ok=True, parents=True)
        self.qaa_dir.mkdir(exist_ok=True, parents=True)

    @property
    def name(self) -> str:
        return "sort"
    
    @property
    def description(self) -> str:
        return "Sort files from input directory to corresponding subdirectories"

    def setup_parser(self, parser):
        parser.add_argument(
            '--dry-run',
            action='store_true',
            help='Simulate sorting without actual file moving'
        )

    def execute(self, args):
        counters = {
            'cpp': 0,
            'json': 0,
            'qaa': 0,
            'skipped': 0
        }

        for file_path in self.path_config.sources_dir.rglob('*'):
            if not file_path.is_file():
                continue

            try:
                if self._is_cpp_file(file_path):
                    self._copy_file(file_path, self.cpp_dir, args.dry_run)
                    counters['cpp'] += 1
                elif self._is_json_file(file_path):
                    self._copy_file(file_path, self.json_dir, args.dry_run)
                    counters['json'] += 1
                elif self._is_qaa_file(file_path):
                    self._copy_file(file_path, self.qaa_dir, args.dry_run)
                    counters['qaa'] += 1
                else:
                    counters['skipped'] += 1
            except Exception as e:
                print(f"[ERROR] Failed to process {file_path.name}: {str(e)}")
                counters['skipped'] += 1

        print("\n[RESULT] Sorting completed:")
        print(f"  C++ files:    {counters['cpp']}")
        print(f"  JSON files:   {counters['json']}")
        print(f"  Q&A files:    {counters['qaa']}")
        print(f"  Skipped files: {counters['skipped']}")

    def _is_cpp_file(self, file_path: Path) -> bool:
        return file_path.suffix[1:] in self.vector_config.file_extensions_cpp

    def _is_json_file(self, file_path: Path) -> bool:
        if file_path.suffix.lower() not in self.vector_config.file_extensions_json:
            return False
            
        try:
            with open(file_path, 'r', encoding='utf-8') as f:
                content = f.read(100)
                return '{' in content
        except UnicodeDecodeError:
            return False

    def _is_qaa_file(self, file_path: Path) -> bool:
        if file_path.suffix.lower() not in self.vector_config.file_extensions_qaa:
            return False

        try:
            with open(file_path, 'r', encoding='utf-8') as f:
                lines = [line.strip() for line in [f.readline(), f.readline()]]
                return lines[0].startswith("Q:") and lines[1].startswith("A:")
        except (UnicodeDecodeError, IndexError):
            return False

    def _move_file(self, src: Path, dest_dir: Path, dry_run: bool):
        dest = dest_dir / src.name
        action = "Would move" if dry_run else "Moving"
        print(f"[{action}] {src.name} => {dest_dir.name}/")
        
        if not dry_run:
            shutil.move(str(src), str(dest))

    def _copy_file(self, src: Path, dest_dir: Path, dry_run: bool):
        dest = dest_dir / src.name
        action = "Would copy" if dry_run else "Copying"
        print(f"[{action}] {src.name} => {dest_dir.name}/")
        
        if not dry_run:
            shutil.copy2(src, dest)  # copy2 сохраняет метаинформацию (время создания и т.д.)

    def _check_preconditions(self):
        return self.state.state == "configured"    #Пытаюсь отслеживать поток команд.... Как то выглядит нелепо...