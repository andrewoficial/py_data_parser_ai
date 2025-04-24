# commands/cmd_interactive.py
from prompt_toolkit import PromptSession
from prompt_toolkit.history import FileHistory
from prompt_toolkit.auto_suggest import AutoSuggestFromHistory
from commands.base import BaseCommand
from pathlib import Path
import argparse

class interactive_mode(BaseCommand):
    def __init__(self, project_root: Path):
        self.project_root = project_root
        self.session = PromptSession(
            history=FileHistory('.cmd_history'),
            auto_suggest=AutoSuggestFromHistory()
        )
        self._register_autocompletion()

    @property
    def name(self) -> str:
        return "interactive"
    
    @property
    def description(self) -> str:
        return "Run interactive mode"        

    def _register_autocompletion(self):
        from prompt_toolkit.completion import WordCompleter
        self.completer = WordCompleter([
            'initial',
            'clear',
            'help',
            'exit',
            'interactive'
        ], ignore_case=True)

    def _run_interactive(self):
        print("Interactive mode (type 'help' for commands)")
        while True:
            try:
                user_input = self.session.prompt(
                    "> ",
                    completer=self.completer,
                    complete_while_typing=True
                ).strip()
                
                if user_input.lower() in ['exit', 'quit']:
                    break
                    
                self._process_command(user_input)
                
            except (KeyboardInterrupt, EOFError):
                print("\nExiting interactive mode...")
                break

    def _process_command(self, input_str):
        from interfaces.cli import CLI
        cli = CLI(interactive_mode=True)
        
        # Парсинг команды
        import shlex
        parts = shlex.split(input_str)
        if not parts:
            return
        
        # Имитация системных аргументов
        import sys
        original_argv = sys.argv
        try:
            sys.argv = ['main.py'] + parts
            cli.run()
        finally:
            sys.argv = original_argv

#Имплементация наследования
    def setup_parser(self, parser: argparse.ArgumentParser):
        pass

    def execute(self, args):
        print("Инициализация...")
        self.state.set_state("ready")         
        print("Switch to interactive mode")
        self._run_interactive()

    def _check_preconditions(self):
        return self.state.state == "uninitialized"          
