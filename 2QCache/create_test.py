from enum import Enum
from random import randint


class Queues(Enum):
    IN = 1
    OUT = 2
    HOT = 3
    NONE = 4

    def __repr__(self) -> str:
        return str(self.name)


class TwoQCache:
    def __init__(self, in_, out, hot) -> None:
        self.in_size = in_
        self.out_size = out
        self.hot_size = hot
        self.in_ = []
        self.out = []
        self.hot = []

    def get_elem_queue(self, elem):
        if elem in self.in_:
            return Queues.IN
        elif elem in self.out:
            return Queues.OUT
        elif elem in self.hot:
            return Queues.HOT
        else:
            return Queues.NONE

    def insert_elem(self, elem):
        queue = self.get_elem_queue(elem)
        if queue == Queues.OUT:
            if len(self.hot) == self.hot_size:
                del self.hot[-1]
            self.hot.insert(0, elem)
            del self.out[self.out.index(elem)]

        elif queue == Queues.NONE:
            if self.in_size == len(self.in_):
                if self.out_size == len(self.out):
                    del self.out[-1]
                self.out.insert(0, self.in_[-1])
                del self.in_[-1]
            self.in_.insert(0, elem)

    def clear(self):
        self.in_ = []
        self.out = []
        self.hot = []

    def show_queues(self):
        print(f"IN: {self.in_}")
        print(f"OUT: {self.out}")
        print(f"HOT: {self.hot}")
        print("=============")


class TwoQCacheTest:
    def __init__(self, in_size, out_size, hot_size) -> None:
        self.cache = TwoQCache(in_size, out_size, hot_size)
        self.in_hits = 0
        self.out_hits = 0
        self.hot_hits = 0
        self.misses = 0

    def clear(self):
        self.cache.clear()
        self.in_hits = 0
        self.out_hits = 0
        self.hot_hits = 0
        self.misses = 0

    def warm_cahce(self, data):
        for elem in data:
            self.cache.insert_elem(elem)

    def test(self, data):
        for elem in data:
            q = self.cache.get_elem_queue(elem)
            self.cache.insert_elem(elem)
            if q == Queues.IN:
                self.in_hits += 1
            elif q == Queues.OUT:
                self.out_hits += 1
            elif q == Queues.HOT:
                self.hot_hits += 1
            else:
                self.misses += 1
        total = self.hot_hits + self.in_hits + self.out_hits + self.misses
        print(
            f"in hits: {self.in_hits};\nout hits: {self.out_hits};\nhot hits: {self.hot_hits};\nratio: {(self.in_hits + self.hot_hits) / total}"
        )
        return self.in_hits, self.out_hits, self.hot_hits, total


def random_test(
    i, o, h, upper_bound: int, lower_bound: int, test_size: int, test_file: str
):
    f = open(test_file, "w")
    cache_test = TwoQCacheTest(i, o, h)

    # prepare warmup data
    warmup = []
    _ = [randint(lower_bound, upper_bound) for __ in range(o)]
    warmup.extend(_ + _)
    warmup.extend(randint(lower_bound, upper_bound) for __ in range(i + o))
    cache_test.warm_cahce(warmup)

    # prepare test data
    test = [randint(lower_bound, upper_bound) for __ in range(test_size)]
    inh, outh, hoth, tot = cache_test.test(test)
    cache_test.clear()
    if tot != test_size:
        f.close()
        raise Exception("some shit happen")

    print(i, o, h, len(warmup), len(test), *warmup, *test, inh, outh, hoth, tot, file=f)
    f.close()


def test_seq_scan():
    a, b, c = 5, 20, 10
    s = 0
    cache_test = TwoQCacheTest(a, b, c)

    # warmup data
    warmup = []
    warmup.extend(range(s, b))
    s += b
    warmup.extend(range(s, s + a))
    s += a
    warmup.extend(range(b))
    # промываем кэш
    warmup.extend(range(1000, 1000 + a + b))
    # in hot nums [0, b], in in + out nums [s, b] + [s, a]
    cache_test.warm_cahce(warmup)
    cache_test.cache.show_queues()

    test = list(range(b))
    cache_test.test(test)


def hand_writed():
    i, o, h = 5, 5, 5
    # проверка на вытеснение ключей из in
    TwoQCacheTest(i, o, h).test(list(range(10, 0, -1)))  # 0, 0, 0
    # проверка на вытеснение ключей из out в hot и из hot
    TwoQCacheTest(i, o, h).test(
        list(range(1, 11))
        + list(range(1, 6))
        + list(range(11, 16))
        + list(range(6, 11))
    )  # 0, 10, 0
    # проверка на крайний случай маленьких кэшей
    TwoQCacheTest(1, 1, 1).test([1, 2, 1, 3, 2, 2, 3])  # 1, 2, 1
    # дефолт проверка с +- случайными данными
    TwoQCacheTest(i, o, h).test(
        [
            *range(10),
            *range(5),
            *range(10, 15),
            2,
            3,
            5,
            9,
            7,
            16,
            17,
            13,
            5,
            9,
            *range(18, 28),
            9,
            7,
            10,
            11,
            14,
            17,
            26,
            4,
            25,
            1,
            2,
            3,
        ]
    )  # 1, 4, 8


if __name__ == "__main__":
    sets = [
        (1, 1, 1, 0, 5, 100),  # тест с маленьким кэшем
        (2, 20, 5, 0, 30, 10000),  # тест с большрим out
        (10, 1, 10, 0, 30, 10000),  # тест с маленьким out
        (2, 4, 50, 0, 30, 10000),  # тест с большим hot
        (
            5,
            10,
            20,
            1,
            40,
            10000,
        ),  # дефолтный тест с маленьким количетсвом данных для проверки инвариантов
        (5, 10, 20, 1, 40, 70000),  # дефолтный тест
        (50, 100, 200, 0, 500, 1000000),  # большой тест на большой кэш
        (10, 20, 30, 1, 10000, 100000),  # тест на большой разброс данных
        (10, 50, 40, 1, 100, 10000),  # еще один дефолтный тест
    ]
    for u in range(len(sets)):
        print(f"test {u}\n==============")
        i, o, h, low, upp, size = sets[u]
        filename = f"tests/test_file{u}.test"
        random_test(i, o, h, upp, low, size, filename)
