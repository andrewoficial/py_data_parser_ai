# error_types.py
from enum import Enum

class ErrorLevel(Enum):
    NOTICE = "NOTICE"
    WARNING = "WARNING"
    ERROR = "ERROR"

class ValidationError:
    def __init__(self, 
                 error_level: ErrorLevel,
                 file: str,
                 message: str,
                 element: int = None,
                 line: int = None):
        self.error_level = error_level
        self.file = file
        self.element = element
        self.line = line
        self.message = message

    def to_dict(self):
        return {
            'type': self.error_level.value,
            'file': self.file,
            'element': self.element,
            'line': self.line,
            'message': self.message
        }