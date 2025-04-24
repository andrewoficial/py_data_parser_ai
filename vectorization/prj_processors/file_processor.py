from pathlib import Path
from hashlib import md5
import json
from prj_config import Config
import time

class FileProcessor:
    def __init__(self, config: Config):
        self.config = config
        print("[FileProcessor] Initialized with config:")
        print(f"  • Source dir: {self.config.SOURCES_DIR}")
        print(f"  • Ignored dirs: {', '.join(self.config.IGNORE_DIRS)}")
        print(f"  • Allowed extensions: {', '.join(self.config.FILE_EXTENSIONS)}")
        print(f"  • Max file size: {self.config.MAX_FILE_SIZE/1024/1024:.1f} MB\n")

    def collect_files(self):
        print("[FileCollector] Scanning source directory...")
        start_time = time.time()
        file_count = 0
        
        for file_path in self.config.SOURCES_DIR.rglob('*'):
            if self._should_process(file_path):
                file_count += 1
                print(f"  ✓ Found: {file_path.name} ({file_path.stat().st_size/1024:.1f} KB)")
                yield file_path
                
        print(f"\n[FileCollector] Scan complete! Found {file_count} files")
        print(f"  Time elapsed: {time.time() - start_time:.2f} seconds\n")

    def _should_process(self, file_path: Path):
        result = (
            file_path.suffix[1:] in self.config.FILE_EXTENSIONS and
            not any(part in self.config.IGNORE_DIRS for part in file_path.parts) and
            file_path.is_file() and
            file_path.stat().st_size < self.config.MAX_FILE_SIZE
        )
        
        if not result:
            reason = self._get_rejection_reason(file_path)
            print(f"  ! Skipped: {file_path.name} ({reason})")
            
        return result

    def _get_rejection_reason(self, file_path: Path) -> str:
        if file_path.suffix[1:] not in self.config.FILE_EXTENSIONS:
            return f"unsupported extension ({file_path.suffix})"
        if any(part in self.config.IGNORE_DIRS for part in file_path.parts):
            return "in ignored directory"
        if not file_path.is_file():
            return "not a file"
        if file_path.stat().st_size >= self.config.MAX_FILE_SIZE:
            size_mb = file_path.stat().st_size/1024/1024
            return f"too large ({size_mb:.1f} MB)"
        return "unknown reason"

    def get_file_hashes(self, files):
        print("\n[FileHasher] Calculating file hashes...")
        start_time = time.time()
        hashes = {}
        
        for file in files:
            file_hash = md5(file.read_bytes()).hexdigest()
            hashes[str(file)] = file_hash
            print(f"  • {file.name}: {file_hash[:8]}...")
            
        print(f"\n[FileHasher] Hashes calculated for {len(hashes)} files")
        print(f"  Time elapsed: {time.time() - start_time:.2f} seconds\n")
        return hashes

    def get_changed_files(self, current_hashes):
        print("[ChangeDetector] Checking for file changes...")
        
        try:
            with open(self.config.CACHE_PATH, 'r') as f:
                old_hashes = json.load(f)
                
            changed_files = []
            for file_path, current_hash in current_hashes.items():
                old_hash = old_hashes.get(file_path)
                
                if old_hash != current_hash:
                    changed_files.append(Path(file_path))
                    status = "modified" if file_path in old_hashes else "new"
                    print(f"  • {Path(file_path).name}: {status} ({current_hash[:8]}...)")
                else:
                    print(f"  • {Path(file_path).name}: unchanged")
                    
            print(f"\n[ChangeDetector] Found {len(changed_files)} changed files")
            return changed_files
            
        except FileNotFoundError:
            print("  ! No previous hashes found, treating all files as changed")
            return [Path(f) for f in current_hashes.keys()]