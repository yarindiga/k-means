#define PY_SSIZE_T_CLEAN
#include <Python.h>
#include <math.h>
 
#define MAX_ITER(argc, argv) argc == 4 ? 200 : argv[2]
#define ERROR_OCCURED "An Error Has Occured"
#define INVALID_INPUT "Invalid Input!"

static void printerr(char *msg){
    printf("%s\n", msg);
    exit(1);
}

static void free_double_pointer(double **dp, int k)
{
    int i;

    for (i = 0; i < k; i++) free(dp[i]);
    free(dp);
}

static void free_data(double **points, double **centroids, double **clusters_sums, int *clusters_counts, int k, int n_rows)
{
    free_double_pointer(centroids, k);
    free_double_pointer(clusters_sums, k);
    free_double_pointer(points, n_rows);
    free(clusters_counts);
}


static int update_centroids(double **centroids, double **clusters_sums, int *clusters_count, int k, int v_len, double epsilon)
{
    int i, j, change_less_than_epsilon = 1;
    double old_acc = 0, new_acc = 0;

    for (i = 0; i < k; i++) {
        for (j = 0; j < v_len; j++) {
            old_acc += pow(centroids[i][j], 2);
            new_acc += pow(clusters_sums[i][j] / clusters_count[i], 2);
        }
        old_acc = sqrt(old_acc);
        new_acc = sqrt(new_acc);
        if (fabs(old_acc - new_acc) >= epsilon) {
            change_less_than_epsilon = 0;
            break;
        }
    }
    if (!change_less_than_epsilon) {
        for (i = 0; i < k; i++) 
            for (j = 0; j < v_len; j++) 
                centroids[i][j] = clusters_sums[i][j] / clusters_count[i];
            
        for (i = 0; i < k; i++) {
            for (j = 0; j < v_len; j++)
                clusters_sums[i][j] = .0;
            clusters_count[i] = 0;
        }
    }
    return change_less_than_epsilon;
}

static double distance(double *p1, double *p2, int len)
{
    double d = 0;
    int i;

    for (i = 0; i < len; i++)
        d += (p1[i] - p2[i]) * (p1[i] - p2[i]);
    
    return d;
}

static int closest_cluster(double *point, int v_len, double ** centroids, int k)
{
    int i, min_index = 0;
    double min_dist = distance(centroids[0], point, v_len), curr_dist;

    for (i = 1; i < k; i++) {
        curr_dist = distance(centroids[i], point, v_len);
        if (min_dist > curr_dist) {
            min_dist = curr_dist;
            min_index = i;
        }
    }
    return min_index;
}


static double **initialize_matrix(int n_rows, int n_cols)
{
    double **matrix;
    int i;

    matrix = (double **)calloc(n_rows, sizeof(double *));
    for (i = 0; i < n_rows; i++) {
        matrix[i] = (double *) calloc(n_cols, sizeof(double));
        if (matrix[i] == NULL) {
            // printf("ON LINE 199\n");
            printerr(ERROR_OCCURED);

        }
    }
    return matrix;
}

static double **matrix_py_to_native(PyObject *py_matrix)
{
    double **matrix;
    Py_ssize_t n_rows, n_cols = 0;
    int i, j;
    PyObject *list;

    if (!PyList_Check(py_matrix))
        return NULL;
    n_rows = PyList_Size(py_matrix);
    if (n_rows) n_cols = PyList_Size(PyList_GetItem(py_matrix, 0));

    matrix = initialize_matrix(n_rows, n_cols);
    for (i = 0; i < n_rows; i++) {
        list = PyList_GetItem(py_matrix, i);
        for (j = 0; j < n_cols; j++) {
            matrix[i][j] = PyFloat_AsDouble(PyList_GetItem(list, j));
            // printf("matrix[%d][%d] = %f\n", i, j, matrix[i][j]);
            if (matrix[i][j]  == -1.0 && PyErr_Occurred()) {
                // free_data(matrix, clusters_sums, clusters_counts, k);
                fprintf(stderr, ERROR_OCCURED);
                // printf("ERROR LINE 224\n");
                return NULL;
            }
        }
    }
    return matrix;
}

static PyObject *fit_api(PyObject *self, PyObject *args)
{
    Py_ssize_t points_count;
    int is_converged = 1, cluster_index;
    double **points, epsilon;
    int max_iter, v_len = 0, k;

    double **centroids;
    double **clusters_sums;
    int *clusters_counts;
    int i, j, counter = 0;
    PyObject *pylist, *pycent, *py_initial_centroids, *py_points;
    if (!PyArg_ParseTuple(args, "OOiid", &py_initial_centroids, &py_points, &k, &max_iter, &epsilon))
        return NULL;
    if (!PyList_Check(py_initial_centroids) || !PyList_Check(py_points))
        return NULL;
    points_count = PyList_Size(py_points);
    if (points_count) v_len = PyList_Size(PyList_GetItem(py_points, 0));
 
    clusters_counts = (int *)calloc(k, sizeof(int));
    if (clusters_counts == NULL) {
        printerr(ERROR_OCCURED);
    }
    clusters_sums = initialize_matrix(k, v_len);
    centroids = matrix_py_to_native(py_initial_centroids);
    points = matrix_py_to_native(py_points);
    for (counter = 0; counter < max_iter; counter++) {
        for (i = 0; i < points_count; i++) {
            cluster_index = closest_cluster(points[i], v_len, centroids, k);
            clusters_counts[cluster_index]++; 
            for (j = 0; j < v_len; j++) {
                clusters_sums[cluster_index][j] += points[i][j];
            }            
        }
        
        is_converged = update_centroids(centroids, clusters_sums, clusters_counts, k, v_len, epsilon);
        if (is_converged)
            break;
    }
    pylist = PyList_New(k);
    for (i = 0; i < k; i++) {
        pycent = PyList_New(v_len);
        for (j = 0; j < v_len; j++) 
            PyList_SetItem(pycent, j, Py_BuildValue("d", centroids[i][j]));
        PyList_SetItem(pylist, i, pycent);
    }
    free_data(points, centroids, clusters_sums, clusters_counts, k, points_count);
    return pylist;
}


static PyMethodDef kmeans_methods[] = {
    {"fit",
    (PyCFunction) fit_api,
    METH_VARARGS,
    PyDoc_STR("A KMeans implementation")},
    {NULL, NULL, 0, NULL}
};

static struct PyModuleDef moduledef = {
    PyModuleDef_HEAD_INIT,
    "mykmeanssp",
    NULL,
    -1,
    kmeans_methods
};


PyMODINIT_FUNC PyInit_mykmeanssp(void)
{
    PyObject *m;
    m = PyModule_Create(&moduledef);
    return m;
}
