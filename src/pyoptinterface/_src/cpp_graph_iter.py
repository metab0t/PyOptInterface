from collections import namedtuple
from .nlcore_ext import cpp_graph_cursor

cpp_graph_instruction = namedtuple("cpp_graph_instruction", ["op", "args", "n_result"])


class cpp_graph_iterator:
    def __init__(self, graph):
        self.graph = graph
        self.init = False
        self.cursor = None
        self.N = graph.n_operator

    def __iter__(self):
        return self

    def __next__(self):
        if self.N == 0:
            raise StopIteration

        graph = self.graph
        if not self.init:
            self.init = True
            self.cursor = cpp_graph_cursor()
        else:
            if self.cursor.op_index >= self.N:
                raise StopIteration

        cursor = self.cursor

        op = graph.get_cursor_op(cursor)
        args = graph.get_cursor_args(cursor)
        n_result = 1

        instruction = cpp_graph_instruction(op, args, n_result)

        graph.next_cursor(self.cursor)

        return instruction
