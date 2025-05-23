# interfaces/cli.py
import argparse
from pathlib import Path
from commands.registry import CommandRegistry

class CLI:
    def __init__(self, interactive_mode=False):

        self.interactive_mode = interactive_mode

        self.project_root = Path(__file__).parent.parent
        self.registry = CommandRegistry(self.project_root)
        self.commands = self._init_commands()
        self.parser = self._create_root_parser()

    def _init_commands(self):
        commands = self.registry.instantiate_commands()
        # Инжектим реестр в команду help
        if "show_help" in commands:
            commands["show_help"].registry = self.registry
        return commands
        
    def _create_root_parser(self):
        parser = argparse.ArgumentParser(
            description="Project CLI Tool",
            formatter_class=argparse.RawTextHelpFormatter,
            add_help=False
        )
        subparsers = parser.add_subparsers(
            title='Available commands',
            dest='command',
            metavar='COMMAND'
        )
        
        for name, command in self.commands.items():
            cmd_parser = subparsers.add_parser(
                name,
                help=command.description,
                formatter_class=argparse.ArgumentDefaultsHelpFormatter
            )
            command.setup_parser(cmd_parser)
            cmd_parser.set_defaults(handler=command.execute)
        
        return parser

    def run(self):
        args = self.parser.parse_args()

        if self.interactive_mode and not args.command:
            return  # Пропускаем в интерактивном режиме

        if not args.command:
            print("not args.command")
            self._print_help()
            return            
        
        if hasattr(args, 'handler'):
            args.handler(args)
        else:
            print(f"Command '{args.command}' not implemented!")
            self._print_help()

    def _print_help(self):
        self.parser.print_help()
        