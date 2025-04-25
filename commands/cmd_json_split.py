# commands/cmd_process_json.py
import json
import shutil
from pathlib import Path
from typing import List, Tuple, Generator
from commands.base import BaseCommand
from core.settings import get_path_config, get_vector_config
from core.error_types import ValidationError
from core.json_validator import JsonValidator  # Предполагается, что это ваш валидатор

class SplitJsonCommand(BaseCommand):
    def __init__(self, project_root: Path):
        self.path_config = get_path_config()
        self.vector_config = get_vector_config()
        self.validator = JsonValidator()
        self._setup_directories()

    @property
    def name(self) -> str:
        return "json_split"
    
    @property
    def description(self) -> str:
        return "Process JSON files: split and validate pieces"

    def _setup_directories(self):
        self.path_config.json_split_dir.mkdir(exist_ok=True, parents=True)
        self.path_config.json_syntax_ok.mkdir(exist_ok=True, parents=True)
        self.path_config.json_syntax_err.mkdir(exist_ok=True, parents=True)

    def _check_preconditions(self):
        return self.state.state == "uninitialized"

    def setup_parser(self, parser):
        parser.add_argument(
            '--max-size',
            type=int,
            default=1,
            help='Maximum size of JSON parts in MB (default: 1)'
        )
        parser.add_argument(
            '--dry-run',
            action='store_true',
            help='Simulate processing without actual changes'
        )

    def execute(self, args):
        print("[PROCESSING] Starting JSON processing...")
        
        for json_file in self.path_config.json_dir.glob('**/*.json'):
            print(f"\nProcessing file: {json_file.name}")
            success, errors = self._process_file(json_file, args.max_size, args.dry_run)
            
            if success:
                print(f"[SUCCESS] File processed successfully")
            else:
                print(f"[ERROR] File processing failed with {len(errors)} errors")

    def _process_file(self, file_path: Path, max_size_mb: int, dry_run: bool) -> Tuple[bool, List[ValidationError]]:
        all_errors = []
        has_critical_errors = False

        try:
            # Split file
            parts = list(self._split_json(file_path, max_size_mb, dry_run))
            


            return not has_critical_errors, all_errors

        except Exception as e:
            error = ValidationError(
                error_level=ErrorLevel.ERROR,
                file=str(file_path),
                message=f"Processing failed: {str(e)}"
            )
            return False, [error]

    def _split_json(self, file_path: Path, max_size_mb: int, dry_run: bool) -> Generator[Path, None, None]:
        max_size = max_size_mb * 1024 * 1024
        part_num = 1
        base_name = file_path.stem

        with open(file_path, 'r', encoding='utf-8') as f:
            data = json.load(f)

        if not isinstance(data, list):
            raise ValueError("Root JSON element must be an array")

        current_chunk = []
        current_size = 0

        for item in data:
            item_size = len(json.dumps(item, ensure_ascii=False).encode('utf-8'))
            
            if current_size + item_size > max_size:
                part_path = self._save_part(base_name, part_num, current_chunk, dry_run)
                yield part_path
                part_num += 1
                current_chunk = []
                current_size = 0

            current_chunk.append(item)
            current_size += item_size

        if current_chunk:
            part_path = self._save_part(base_name, part_num, current_chunk, dry_run)
            yield part_path

    def _save_part(self, base_name: str, part_num: int, data: list, dry_run: bool) -> Path:
        part_path = self.path_config.json_split_dir / f"{base_name}_part_{part_num}.json"
        
        if not dry_run:
            with open(part_path, 'w', encoding='utf-8') as f:
                json.dump(data, f, indent=2, ensure_ascii=False)
        
        print(f"{'[DRY RUN] ' if dry_run else ''}Created part: {part_path.name}")
        return part_path

