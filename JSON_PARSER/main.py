# main.py call first contain main method
import sys
from pathlib import Path
from processing import process_file
from error_types import ValidationError, ErrorLevel

def print_errors(errors: list[ValidationError]):
    error_groups = {
        ErrorLevel.ERROR: [],
        ErrorLevel.WARNING: [],
        ErrorLevel.NOTICE: []
    }

    for error in errors:
        error_groups[error.error_level].append(error)

    for level in [ErrorLevel.ERROR, ErrorLevel.WARNING, ErrorLevel.NOTICE]:
        if error_groups[level]:
            print(f"\n--- {level.value}S ({len(error_groups[level])}) ---")
            for error in error_groups[level]:
                d = error.to_dict()
                print(f"  • File: {d['file']}")
                if d['element']: print(f"    Element: {d['element']}")
                if d['line']: print(f"    Line: {d['line']}")
                print(f"    Message: {d['message']}")

def main():
    if len(sys.argv) != 2:
        print("Usage: python main.py <path_to_json>")
        sys.exit(1)

    file_path = Path(sys.argv[1])
    
    if not file_path.exists():
        print(f"File not found: {file_path}")
        sys.exit(1)

    print(f"Processing: {file_path}")
    is_valid, errors = process_file(file_path)

    if is_valid:
        print("\n✓ File is valid")
        if errors:
            print_errors(errors)
        sys.exit(0)
    else:
        print("\n✗ Validation failed")
        print_errors(errors)
        sys.exit(1)

if __name__ == "__main__":
    main()