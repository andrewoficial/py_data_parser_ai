# commands/cmd_check_files.py
from pathlib import Path
from hashlib import md5
import json
import time
from commands.base import BaseCommand
from core.settings import get_path_config, get_vector_config

class CheckFilesCommand(BaseCommand):
    def __init__(self, project_root: Path):
        self.project_root = project_root
        self.path_config = get_path_config()
        self.vector_config = get_vector_config()
        
        self.file_extensions = self.vector_config.supported_extensions
        self.ignore_dirs = self.vector_config.ignore_dirs
        self.max_file_size = self.vector_config.max_file_size

    @property
    def name(self) -> str:
        return "check"
    
    @property
    def description(self) -> str:
        return "Check for new or modified files in source directories"

    def setup_parser(self, parser):
        parser.add_argument(
            '--force',
            action='store_true',
            help='Force full rescan ignoring cache'
        )

    def execute(self, args):
        print(f"[SCAN] Starting filesystem scan in {self.path_config.sources_dir}")
        files = self._collect_files()
        
        print("\n[HASH] Calculating file hashes...")
        current_hashes = self._get_file_hashes(files)
        
        print("\n[DIFF] Checking for changes:")
        changed_files = self._get_changed_files(current_hashes, args.force)
        
        if changed_files:
            print("\n[RESULT] New files found! Updating hashes...")
            self._save_hashes(current_hashes)
        else:
            print("\n[RESULT] No changes detected")
            
        return changed_files

    def _collect_files(self):
        start_time = time.time()
        valid_files = []
        
        for file_path in self.path_config.sources_dir.rglob('*'):
            if self._should_process(file_path):
                valid_files.append(file_path)
                print(f"  [OK] {file_path.relative_to(self.path_config.sources_dir)}")
            else:
                reason = self._get_rejection_reason(file_path)
                print(f"  [SKIP] {file_path.name} ({reason})")
                
        print(f"\n[SCAN] Found {len(valid_files)} valid files")
        print(f"  Time: {time.time() - start_time:.2f}s")
        return valid_files

    def _should_process(self, file_path: Path):
        if not file_path.is_file():
            return False
        if file_path.suffix not in self.file_extensions:
            return False
        if any(part in self.ignore_dirs for part in file_path.parts):
            return False
        if file_path.stat().st_size > self.max_file_size:
            return False
        return True

    def _get_rejection_reason(self, file_path: Path):
        print(file_path)
        if not file_path.is_file():
            return "not a file"
        if file_path.suffix not in self.file_extensions:
            return f"unsupported extension {file_path.suffix}"
        if any(part in self.ignore_dirs for part in file_path.parts):
            return "in ignored directory"
        if file_path.stat().st_size > self.max_file_size:
            #print(self.max_file_size)
            #print(file_path.stat().st_size)
            return "file too large"
        return "unknown reason"

    def _get_file_hashes(self, files):
        hashes = {}
        start_time = time.time()
        
        for file in files:
            try:
                file_hash = md5(file.read_bytes()).hexdigest()
                rel_path = str(file.relative_to(self.path_config.sources_dir))
                hashes[rel_path] = file_hash
                print(f"  [HASH] {file.name}: {file_hash[:8]}")
            except Exception as e:
                print(f"  [ERROR] Failed to hash {file.name}: {str(e)}")
        
        print(f"\n[HASH] Processed {len(hashes)} files")
        print(f"  Time: {time.time() - start_time:.2f}s")
        return hashes

    def _get_changed_files(self, current_hashes, force):
        changed = []
        try:
            if force or not self.path_config.cache_path.exists():
                raise FileNotFoundError
                
            with open(self.path_config.cache_path, 'r') as f:
                old_hashes = json.load(f)
                
            for rel_path, new_hash in current_hashes.items():
                old_hash = old_hashes.get(rel_path)
                
                if not old_hash:
                    changed.append(rel_path)
                    print(f"  [NEW] {rel_path}")
                elif old_hash != new_hash:
                    changed.append(rel_path)
                    print(f"  [MODIFIED] {rel_path}")
                    
        except FileNotFoundError:
            print("  [INIT] No cache found, treating all files as new")
            changed = list(current_hashes.keys())
            
        return changed

    def _save_hashes(self, hashes):
        self.path_config.cache_dir.mkdir(exist_ok=True, parents=True)
        with open(self.path_config.cache_path, 'w') as f:
            json.dump(hashes, f, indent=2)
        print(f"[CACHE] Saved {len(hashes)} hashes to {self.path_config.cache_path}")

    def _check_preconditions(self):
        return True