class SystemState:
    _instance = None

    def __new__(cls):
        if cls._instance is None:
            cls._instance = super(SystemState, cls).__new__(cls)
            cls._instance._state = "uninitialized"
        return cls._instance

    @property
    def state(self):
        return self._state

    def set_state(self, new_state):
        allowed_transitions = {
            "uninitialized": ["ready"],
            "ready": ["processing", "ready"],
            "processing": ["ready"]
        }
        if new_state in allowed_transitions[self._state]:
            self._state = new_state
            return True
        return False