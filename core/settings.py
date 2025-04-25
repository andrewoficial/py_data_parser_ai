from pathlib import Path
from configs.conf import PathConfig, VectorConfig, DBConfig, ModelConfig

# Глобальные переменные конфигурации
path_config: PathConfig = None
vector_config: VectorConfig = None
db_config: DBConfig = None
model_config: ModelConfig = None


class SettingsLoader:
    @staticmethod
    def load_all(project_root: Path = None):
        global path_config, vector_config, db_config, model_config

        path_config = PathConfig()
        vector_config = VectorConfig()
        db_config = DBConfig()
        model_config = ModelConfig()

        print("[OK] Загружены настройки конфигурации:")
        print(f"    base_dir: {path_config.base_dir}")
        print(f"    model: {model_config.model}")
        print(f"    db_host: {db_config.host}")