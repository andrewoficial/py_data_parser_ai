from pathlib import Path
from configs.conf import PathConfig, VectorConfig, DBConfig, ModelConfig

# Приватные кэши для ленивой инициализации
_path_config: PathConfig | None = None
_vector_config: VectorConfig | None = None
_db_config: DBConfig | None = None
_model_config: ModelConfig | None = None


def get_path_config() -> PathConfig:
    global _path_config
    if _path_config is None:
        _path_config = PathConfig()
        print(f"[INIT] path_config loaded: base_dir = {_path_config.base_dir}")
    return _path_config


def get_vector_config() -> VectorConfig:
    global _vector_config
    if _vector_config is None:
        _vector_config = VectorConfig()
        print(f"[INIT] vector_config loaded")
    return _vector_config


def get_db_config() -> DBConfig:
    global _db_config
    if _db_config is None:
        _db_config = DBConfig()
        print(f"[INIT] db_config loaded: host = {_db_config.host}")
    return _db_config


def get_model_config() -> ModelConfig:
    global _model_config
    if _model_config is None:
        _model_config = ModelConfig()
        print(f"[INIT] model_config loaded: model = {_model_config.model}")
    return _model_config


def print_all_configs():
    print("[OK] Загружены настройки конфигурации:")
    print(f"    base_dir: {get_path_config().base_dir}")
    print(f"    model: {get_model_config().model}")
    print(f"    db_host: {get_db_config().host}")

def get_all():
    print("Use carefule: list of parameters updated manually !")
    get_path_config()
    get_model_config()
    get_db_config()
