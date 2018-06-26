#include "pybind11/include/pybind11/pybind11.h"
#include "pybind11/include/pybind11/stl.h"
#include "pybind11/include/pybind11/stl_bind.h"
#include "pybind11/include/pybind11/functional.h"
#include "MCTS.h"
/*
int add(int i[2], int j) {
    return i[1] + j;
}

PYBIND11_MODULE(example, m) {
    m.doc() = "pybind11 example plugin"; // optional module docstring

    m.def("add", &add, "A function which adds two numbers");
}*/

namespace py = pybind11;

PYBIND11_MODULE(mcts,m) {
	py::class_<pos>(m, "pos")
		.def_readwrite("x",&pos::x)
		.def_readwrite("y",&pos::y);

	py::class_<board>(m, "board");

    py::class_<MCTS>(m, "MCTS")
    	.def(py::init<>())
        .def("get_move", &MCTS::get_move),py::arg("myboard"), py::arg("color"),py::arg("depth")=10,py::arg("time_remaining")=0, py::arg("time_opponent")=0;
        

};



