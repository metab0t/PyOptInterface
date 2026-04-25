from pyoptinterface._src.tupledict import (
    flatten_tuple,
    make_tupledict,
    tupledict,
    WILDCARD,
)


def test_flatten_tuple():
    assert list(flatten_tuple((1, (2, 3), (4, 5)))) == [1, 2, 3, 4, 5]
    assert list(flatten_tuple((1, 2, 3))) == [1, 2, 3]
    assert list(flatten_tuple(((1, 2), 3))) == [1, 2, 3]


def test_make_tupledict():
    assert make_tupledict([1, 2], [3, 4], rule=lambda x, y: x * y) == {
        (1, 3): 3,
        (1, 4): 4,
        (2, 3): 6,
        (2, 4): 8,
    }
    assert make_tupledict([1, 2], [3, 4], rule=lambda x, y: None) == {}
    assert make_tupledict([1, 2], rule=lambda x: x) == {1: 1, 2: 2}
    assert make_tupledict([1, 2], [(3, 3), (4, 4)], rule=lambda x, y, z: x) == {
        (1, 3, 3): 1,
        (1, 4, 4): 1,
        (2, 3, 3): 2,
        (2, 4, 4): 2,
    }


def test_tupledict_select():
    # Create a tupledict instance
    td = tupledict(
        {
            (1, 2): "a",
            (1, 3): "b",
            (2, 2): "c",
            (2, 3): "d",
        }
    )

    # Test select with specific keys
    assert list(td.select(1, 2)) == ["a"]
    assert list(td.select(2, WILDCARD)) == ["c", "d"]

    # Test select with wildcard
    assert list(td.select(WILDCARD, 2)) == ["a", "c"]

    # Test select with all wildcards
    assert list(td.select(WILDCARD, WILDCARD)) == ["a", "b", "c", "d"]

    # Test select with non-existing key
    assert list(td.select(3, 2)) == []

    # Test select with key and value
    assert list(td.select(1, 2, with_key=True)) == [((1, 2), "a")]
    assert list(td.select(2, WILDCARD, with_key=True)) == [((2, 2), "c"), ((2, 3), "d")]


def test_tupledict_map():
    td = tupledict([((i, i + 1), i) for i in range(10)])

    td_m = td.map(lambda x: x**2)

    assert isinstance(td_m, tupledict)

    assert list(td_m.values()) == [i**2 for i in range(10)]

    assert list(td_m.keys()) == list(td.keys())


def test_tupledict_cache_invalidation():
    # update() should invalidate select cache
    td = tupledict({(1, 2): "a", (1, 3): "b", (2, 2): "c"})
    assert list(td.select(1, WILDCARD)) == ["a", "b"]
    td.update({(3, 2): "e"})
    assert list(td.select(3, WILDCARD)) == ["e"]

    # pop() should invalidate select cache
    td2 = tupledict({(1, 2): "a", (1, 3): "b"})
    assert list(td2.select(1, WILDCARD)) == ["a", "b"]
    td2.pop((1, 3))
    assert list(td2.select(1, WILDCARD)) == ["a"]

    # clear() should invalidate select cache
    td3 = tupledict({(1, 2): "a"})
    list(td3.select(1, WILDCARD))  # build cache
    td3.clear()
    td3[(1, 2)] = "x"
    assert list(td3.select(1, WILDCARD)) == ["x"]


def test_tupledict_scalar_key_select():
    # tupledict created via __init__ with scalar keys should work with select()
    td = tupledict({1: "a", 2: "b", 3: "c"})
    assert list(td.select(1)) == ["a"]
    assert sorted(td.select(WILDCARD)) == ["a", "b", "c"]
    assert list(td.select(1, with_key=True)) == [(1, "a")]
    assert list(td.select(99)) == []
