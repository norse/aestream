# API Roadmap

```py
class FileInput():
    """
    Load or Iterate Events or Frames from a file

    Supported file formats: aedat, aedat4, dat

    Args:
        file_path (str): Path of file to be used
        resolution (int, int): X,Y resolution of file

    Attributes:
        file_path (str): Path of file to be used
        resolution (int, int): X,Y resolution of file
    """
    def __init__(self, file_path, resolution) -> None:
        pass

    def load(frame_ms=0, device="cpu"):
        """
        Loads the whole file onto device memory

        Args:
            frame_ms (int): Selects data format. When `0`, events are loaded as is.
                Otherwise, events are accumulated into frames with synchronous period of `frame_ms`
            device (str: `cpu`, `cuda`): Selects the device to which the file is loaded

        Returns:
            Array/Tensor containing the contents of the file
        """
        pass
    
    def get_iterator(frame_ms=0, device="cpu", mode="safe", memory_limit=1024): 
        """
        Provides functionality that allows files larger than device memory to be iteratively loaded

        Args:
            frame_ms (int): Selects data format. When `0`, events are loaded as is. Otherwise, events are accumulated into frames with synchronous period of `frame_ms`
            device (str: `cpu`, `cuda`): Selects the device to which the file is loaded.
            mode (str: `safe`, `unsafe`): Selects if slower deterministic (`safe`) or faster nondeterministic (`unsafe`) method is used to create frames. When using `unsafe` the exact frame a event is part of is subject to jitter at runtime.
        
        Returns:
            A generator that loads events or frames to `device` as specified.
        """

class FileInputIterator():


```