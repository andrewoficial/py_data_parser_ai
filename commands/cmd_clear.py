# commands/cmd_clear.py
import argparse
import shutil
from pathlib import Path
from commands.base import BaseCommand

class ClearCommand(BaseCommand):
    def __init__(self, project_root: Path):
        self.project_root = project_root
        self.default_output_dir = project_root / "dir_output"
        self.default_output_dir_tmp = project_root / "dir_temp"
        self.default_output_dir_tmp = project_root / "dir_logs"

    @property
    def name(self) -> str:
        return "clear"
    
    @property
    def description(self) -> str:
        return "Clear output directories"

    def setup_parser(self, parser: argparse.ArgumentParser):
        parser.add_argument(
            '--all',
            action='store_true',
            help='Clear all output directories'
        )
        parser.add_argument(
            '-d', '--dirs',
            nargs='+',
            help='List of directories to clear (default: dir_output in project root)'
        )
        parser.add_argument(
            '--dry-run',
            action='store_true',
            help='Simulate cleaning without actual deletion'
        )

    def execute(self, args):
        # Определяем целевые директории
        target_dirs = []
        
        if args.dirs:
            target_dirs.extend([Path(d) for d in args.dirs])
        else:
            target_dirs.append(self.default_output_dir)
            target_dirs.append(self.default_output_dir_tmp)
        
        # Выводим информацию о текущей рабочей директории
        print(f"\nWorking directory: {Path.cwd()}")
        print(f"Project root: {self.project_root}")
        
        for dir_path in target_dirs:
            # Если путь относительный, делаем его абсолютным относительно корня проекта
            if not dir_path.is_absolute():
                dir_path = self.project_root / dir_path
            
            print(f"\nProcessing directory: {dir_path}")
            
            if not dir_path.exists():
                print(f"[ERR] Directory does not exist: {dir_path}")
                continue
            
            if args.dry_run:
                print("[RUN] Dry run mode - no files will be deleted")
            
            try:
                items = list(dir_path.iterdir())
                if not items:
                    print("[INF] Directory is already empty")
                    continue
                
                print(f"Found {len(items)} items to remove:")
                for item in items:
                    print(f"  - {item.name} ({'dir' if item.is_dir() else 'file'})")
                
                if not args.dry_run:
                    self._clear_directory(dir_path)
                    print(f"[OK] Successfully cleared: {dir_path}")
                    
            except Exception as e:
                print(f"[ERR] Error clearing {dir_path}: {str(e)}")

    def _clear_directory(self, dir_path: Path):
        """Внутренний метод для очистки директории"""
        for item in dir_path.iterdir():
            if item.is_dir():
                shutil.rmtree(item)
            else:
                item.unlink()