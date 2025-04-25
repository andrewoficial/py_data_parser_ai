# commands/registry.py
import importlib
import pkgutil
from pathlib import Path
from typing import Dict, Type
from .base import BaseCommand
import inspect

class CommandRegistry:
    def __init__(self, project_root: Path):
        self.project_root = project_root
        self.command_classes = self._discover_commands()
    
    def _discover_commands(self) -> Dict[str, Type[BaseCommand]]:
        commands = {}
        package = __import__('commands', fromlist=[''])
        
        for _, module_name, _ in pkgutil.iter_modules(package.__path__):
            if module_name.startswith('cmd_'):
                #print("просматриваю " + module_name)
                module = importlib.import_module(f'commands.{module_name}')
                for attr_name in dir(module):

                    attr = getattr(module, attr_name)

                    if (isinstance(attr, type) and 
                        issubclass(attr, BaseCommand) and 
                        attr != BaseCommand):
                        #print(" module_name подходит по условиям" + module_name)
                        # Создаем временный экземпляр для получения имени
                        # Для HelpCommand передаем дополнительный аргумент
                        if attr.__name__ == "HelpCommand":
                            instance = attr(self.project_root, registry=self)
                        else:
                            instance = attr(self.project_root)
                        
                        commands[instance.name] = attr
        #print(" Пул комманд" + str(commands))
        return commands
    
    def instantiate_commands(self) -> Dict[str, BaseCommand]:
        commands = {}
        for name, cmd_class in self.command_classes.items():
            # Проверяем сигнатуру конструктора
            init_signature = inspect.signature(cmd_class.__init__)
            
            # Если конструктор ожидает 'registry' параметр
            if 'registry' in init_signature.parameters:
                commands[name] = cmd_class(self.project_root, registry=self)
            else:
                commands[name] = cmd_class(self.project_root)
        
        return commands