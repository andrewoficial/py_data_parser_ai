# file_splitter.py
import json
from pathlib import Path
from typing import Generator

def split_json_file(file_path: Path, max_size_mb: int = 1) -> Generator[Path, None, None]:
    max_size = max_size_mb * 1024 * 1024
    part_num = 1
    base_name = file_path.stem
    output_dir = file_path.parent

    with open(file_path, 'r', encoding='utf-8') as f:
        data = json.load(f)

    if not isinstance(data, list):
        raise ValueError("Root JSON element must be an array")

    current_chunk = []
    current_size = 0

    for item in data:
        item_size = len(json.dumps(item, ensure_ascii=False).encode('utf-8'))
        print(f"Item size: {item_size / 1024:.2f} KB, Current chunk size: {current_size / 1024 / 1024:.2f} MB")
        
        if current_size + item_size > max_size:
            part_path = output_dir / f"output_dir/{base_name}_part_{part_num}.json"
            with open(part_path, 'w', encoding='utf-8') as f:
                json.dump(current_chunk, f, indent=2, ensure_ascii=False)
            print(f"Saved part {part_num} with {len(current_chunk)} items")
            yield part_path
            part_num += 1
            current_chunk = []
            current_size = 0

        current_chunk.append(item)
        current_size += item_size

    if current_chunk:
        part_path = output_dir / f"output_dir/{base_name}_part_{part_num}.json"
        with open(part_path, 'w', encoding='utf-8') as f:
            json.dump(current_chunk, f, indent=2, ensure_ascii=False)
        print(f"Saved final part {part_num} with {len(current_chunk)} items")
        yield part_path