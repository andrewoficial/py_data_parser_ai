# processing.py call second contain call split_json_file in file_splitter and then validate_json_file in validation
from pathlib import Path
import os
from typing import Tuple, List, Dict
from file_splitter import split_json_file
from validation import validate_json_file
from error_types import ValidationError

def process_file(file_path: Path, max_size_mb: int = 1) -> Tuple[bool, List[ValidationError]]:
    all_errors = []
    total_size = os.path.getsize(file_path)
    has_critical_errors = False

    if total_size > max_size_mb * 1024 * 1024:
        print(f"File is too large ({total_size/1024/1024:.2f} MB), splitting...")
        parts = list(split_json_file(file_path, max_size_mb))
        print(f"Created {len(parts)} parts:")
        for part in parts:
            part_size = os.path.getsize(part) / 1024 / 1024
            print(f"  - {part.name}: {part_size:.2f} MB")
    else:
        parts = [file_path]
        print(f"File size is {total_size/1024/1024:.2f} MB, no splitting needed.")

    for part in parts:
        is_valid, errors = validate_json_file(part)
        all_errors.extend(errors)
        if not is_valid:
            has_critical_errors = True

    return not has_critical_errors, all_errors