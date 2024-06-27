from typing import Iterable
from itertools import product


WILDCARD = "*"


class tupledict(dict):
    def __init__(self, *args, **kwargs):
        super().__init__(*args, **kwargs)
        self.__select_cache = None
        self.__scalar = False

    def __setitem__(self, key, value):
        super().__setitem__(key, value)
        self.__select_cache = None

    def __delitem__(self, key):
        super().__delitem__(key)
        self.__select_cache = None

    def __check_key_length(self):
        if len(self) == 0:
            return
        first_key = next(iter(self.keys()))
        if isinstance(first_key, tuple):
            key_len = len(first_key)
            for k in self.keys():
                if len(k) != key_len:
                    raise ValueError(
                        "The length of keys in tupledict is not consistent"
                    )
            self.__key_len = key_len
            self.__scalar = False
        else:
            key_len = 1
            self.__key_len = key_len
            self.__scalar = True

    def select(self, *keys, with_key=False):
        if len(keys) == 0:
            yield from ()
            return
        if self.__scalar:
            if len(keys) != 1:
                raise ValueError(
                    "The length of keys is not consistent with the scalar tupledict"
                )
            key = keys[0]
            if key == WILDCARD:
                if with_key:
                    yield from self.items()
                else:
                    yield from self.values()
            else:
                if key in self:
                    if with_key:
                        yield key, self[key]
                    else:
                        yield self[key]
                else:
                    yield from ()
        else:
            if self.__select_cache is None:
                self.__check_key_length()
                self.__select_cache = dict()
            key_len = self.__key_len
            if len(keys) > key_len:
                raise ValueError(
                    f"Too many keys for tupledict with {key_len}-tuple keys"
                )
            no_wildcard_indices = tuple(
                i for i, key in enumerate(keys) if key != WILDCARD
            )
            if len(no_wildcard_indices) == 0:
                if with_key:
                    yield from self.items()
                else:
                    yield from self.values()
                return
            no_wildcard_keys = tuple(keys[i] for i in no_wildcard_indices)
            select_cache = self.__select_cache
            indices_cache = select_cache.get(no_wildcard_indices, None)
            if indices_cache is None:
                indices_cache = dict()
                for k, v in self.items():
                    indices = tuple(k[i] for i in no_wildcard_indices)
                    kv_cache = indices_cache.get(indices, None)
                    if kv_cache is None:
                        indices_cache[indices] = [(k, v)]
                    else:
                        kv_cache.append((k, v))
                select_cache[no_wildcard_indices] = indices_cache
            kv_cache = indices_cache.get(no_wildcard_keys, None)
            if kv_cache is None:
                yield from ()
            else:
                if with_key:
                    yield from kv_cache
                else:
                    for k, v in kv_cache:
                        yield v

    def clean(self):
        self.__select_cache = None

    def map(self, func):
        return tupledict((k, func(v)) for k, v in self.items())


def flatten_tuple(t):
    # (1, (2, 3), (4, 5)) -> (1, 2, 3, 4, 5)
    for it in t:
        if isinstance(it, tuple) or isinstance(it, list):
            for element in it:
                yield element
        else:
            yield it


def make_tupledict(*coords: Iterable, rule):
    d = {}
    assert len(coords) > 0
    for coord in product(*coords):
        # (1, (2, 3), (4, 5)) -> (1, 2, 3, 4, 5)
        coord = tuple(flatten_tuple(coord))
        value = rule(*coord)
        if len(coord) == 1:
            coord = coord[0]
        if value is not None:
            d[coord] = value
    return tupledict(d)
