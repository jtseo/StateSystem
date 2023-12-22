// ConsoleUnityTest.cpp : This file contains the 'main' function. Program execution begins and ends there.
//
#include "Python.h"

#include <iostream>

int main()
{
    // Initialize Python interpreter
    Py_Initialize();

    // Import your Python module
    PyObject* pModule = PyImport_ImportModule("pycall");

    if (pModule != NULL) {
        // Get a reference to the Python function
        PyObject* pFunction = PyObject_GetAttrString(pModule, "mydata");

        if (pFunction != NULL && PyCallable_Check(pFunction)) {
            // Create a byte array (you'll need to adapt this to your data)
            const char* byteArray = "your_byte_array_data";
            //PyObject* pArgs = Py_BuildValue("(y#)", byteArray, strlen(byteArray));
            PyObject* pArgs = PyTuple_Pack(1, PyByteArray_FromStringAndSize(byteArray, strlen(byteArray)));

            // Call the Python function with the byte array as parameter
            PyObject* pValue = PyObject_CallObject(pFunction, pArgs);

            // Clean up
            Py_XDECREF(pArgs);
            Py_XDECREF(pValue);
            Py_XDECREF(pFunction);
        }

        Py_XDECREF(pModule);
    }

    // Finalize Python interpreter
    Py_Finalize();
    return 1;
}

// Run program: Ctrl + F5 or Debug > Start Without Debugging menu
// Debug program: F5 or Debug > Start Debugging menu

// Tips for Getting Started: 
//   1. Use the Solution Explorer window to add/manage files
//   2. Use the Team Explorer window to connect to source control
//   3. Use the Output window to see build output and other messages
//   4. Use the Error List window to view errors
//   5. Go to Project > Add New Item to create new code files, or Project > Add Existing Item to add existing code files to the project
//   6. In the future, to open this project again, go to File > Open > Project and select the .sln file
