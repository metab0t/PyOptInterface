from .nlcore_ext import (
    Independent,
    a_double,
    advec,
    ADFun,
)

from typing import Union, Iterable, Optional

# Trace the Python function to construct an ADFun object in CppAD


def trace_adfun_impl(
    f,
    nx: int,
    np: int,
    x_names: Optional[Iterable[str]] = None,
    p_names: Optional[Iterable[str]] = None,
):
    has_parameter = np > 0

    ax_ = advec(a_double() for _ in range(nx))
    if has_parameter:
        ap_ = advec(a_double() for _ in range(np))
        Independent(ax_, ap_)
    else:
        Independent(ax_)

    ax = ax_
    if x_names is not None:
        ax = dict(zip(x_names, ax_))
    if has_parameter:
        ap = ap_
        if p_names is not None:
            ap = dict(zip(p_names, ap_))

    if has_parameter:
        ay = f(ax, ap)
    else:
        ay = f(ax)
    if isinstance(ay, a_double):
        ny = 1
        ay_ = advec([ay])
    else:
        ny = len(ay)
        if isinstance(ay, dict):
            ay_ = advec(y for y in ay.values())
        else:
            ay_ = advec(y for y in ay)
    adf = ADFun()
    adf.Dependent(ax_, ay_)
    adf.optimize("no_compare_op no_conditional_skip no_cumulative_sum_op")

    # check
    assert adf.nx == nx
    assert adf.ny == ny
    assert adf.np == np

    return adf


def trace_adfun(f, x: Union[int, Iterable[str]], p: Union[int, Iterable[str]] = ()):
    if isinstance(x, int):
        nx = x
        x_names = None
    else:
        nx = len(x)
        x_names = x
    if isinstance(p, int):
        np = p
        p_names = None
    else:
        np = len(p)
        p_names = p

    return trace_adfun_impl(f, nx, np, x_names, p_names)
