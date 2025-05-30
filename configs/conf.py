from pydantic_settings import BaseSettings
from pathlib import Path
from typing import Set


class PathConfig(BaseSettings):
    base_dir: Path = Path(__file__).resolve().parent.parent

    sources_dir: Path | None = None
    json_dir: Path | None = None
    json_split_dir: Path | None = None
    vector_store_dir: Path | None = None
    cache_path: Path | None = None

    model_config = {
        "extra": "allow",
        "env_prefix": "path_",
        "env_file": ".env"
    }

    def __init__(self, **kwargs):
        super().__init__(**kwargs)
        self.base_dir = Path(__file__).resolve().parent.parent
        self.sources_dir = self.base_dir / "dir_input" # Файлы для анализа
        self.json_dir = self.base_dir / "dir_temp/inp_json" # Найденные json
        self.cpp_dir = self.base_dir / "dir_temp/inp_cpp" # Найденные cpp
        self.qaa_dir = self.base_dir / "dir_temp/inp_q_and_a" # Найденные q  and a файлы
        self.json_syntax_ok = self.base_dir / "dir_temp/inp_json_syntax_ok" # Найденные json после разделения
        self.json_syntax_err = self.base_dir / "dir_temp/inp_json_syntax_err" # Найденные json после разделения
        self.json_split_dir = self.base_dir / "dir_temp/inp_json_split" # Найденные json после разделения

        self.cache_dir = self.base_dir / "dir_temp/file_hashes" # Хранение хешей
        self.cache_path = self.base_dir / "dir_temp/file_hashes/file_hashes.json" # Хранение файл
        self.unsort_dir = self.base_dir / "dir_temp/inp_unsort" # Не были сортированы
        self.logs_dir = self.base_dir / "dir_logs" # Хранение хешей
        self.vector_store_dir = self.base_dir / "dir_output/summarization" # Хранение обощений


class VectorConfig(BaseSettings):
    chunk_size: int = 256
    overlap: int = 32
    ignore_dirs: Set[str] = {"build", "test", "thirdparty"}
    file_extensions_cpp: Set[str] = {"cpp", "hpp", "h", "cc", "cxx"}
    file_extensions_json: Set[str] = {".json"}
    file_extensions_qaa: Set[str] = {".txt"}
    max_file_size: int = 2 * 9024 * 9024
    supported_extensions: Set[str] = set().union(file_extensions_cpp, file_extensions_json, {"logs"})
    #print(supported_extensions)
    model_config = {
        "extra": "allow",
        "env_prefix": "vector_",
        "env_file": ".env"
    }        


class DBConfig(BaseSettings):
    host: str
    port: int
    name: str
    user: str
    password: str
    sslmode: str = "require"
    model_config = {
        "extra": "allow",
        "env_prefix": "db_",
        "env_file": ".env"
    } 


class ModelConfig(BaseSettings):
    openrouter_api_key: str
    model: str = "meta-llama/llama-3.1-8b-instruct:free"
    ollama_endpoint: str = "http://127.0.0.1:11434/api/generate"
    local_model_name: str = "llama3.2"
    prompt_template: str = (
        "Составь на основе юридического документа JSON с полями 'result' (обвинительный, оправдательный удовлетворено ли ходатайство) и "
        "'base' (статьи на основании которых вынесено решение с пунктами через запятую). Исходные данные: "
    )

    model_config = {
        "extra": "allow",
        "env_prefix": "model_",
        "env_file": ".env"
    } 