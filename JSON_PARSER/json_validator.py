# json_validator.py
from pathlib import Path
from typing import List

class JsonValidator:
    def __init__(self):
        self.line_map = []
        self.raw_data = []

    def build_line_map(self, json_str: str):
        self.line_map = [0]
        for i, char in enumerate(json_str):
            if char == '\n':
                self.line_map.append(i+1)

    def get_line_number(self, pos: int) -> int:
        for line_num, start_pos in enumerate(self.line_map):
            if start_pos > pos:
                return line_num
        return len(self.line_map)