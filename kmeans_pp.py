import numpy as np
import pandas as pd
import argparse
import sys
import os.path
from mykmeanssp import fit


np.random.seed(0)
# np.set_printoptions(precision=4)

def test_filename(filename: str, ext=('.csv', '.txt')):
    if (isinstance(filename, str) and os.path.exists(filename) 
        and filename.endswith(ext)):
        return filename
    else:
        raise argparse.ArgumentError()


def parse_arglist():
    try:
        parser = argparse.ArgumentParser()
        parser.add_argument('k', type=int)
        parser.add_argument('max_iter', type=int, nargs='?', default=200)
        parser.add_argument('eps', type=float)
        parser.add_argument('file_name_1', type=test_filename)
        parser.add_argument('file_name_2', type=test_filename)
        return parser.parse_args()
    except argparse.ArgumentError as argerr:
        print("Invalid argument!")
        print(argerr)
        sys.exit(1)


def select_centroids(data_points, n_centroids):
    # i = 1
    # print(data_points.shape, np.random.choice(data_points.size))
    # data_points.sort_index(inplace=True)

    centroids_indices = np.random.choice(data_points.shape[0], size=1)
    centroids = np.expand_dims(data_points[centroids_indices[0]], axis=0)
    for i in range(1, n_centroids):
        Dls = np.array([min(pow(point - centroid, 2).sum() for centroid in centroids) for point in data_points])
        # D_sum = Dls.sum()
        Pls = Dls / Dls.sum()
        centroids_indices = np.append(centroids_indices, np.random.choice(data_points.shape[0], p=Pls))
        centroids = np.append(centroids, np.expand_dims(data_points[centroids_indices[i]], axis=0), axis=0)
        # i += 1
    return centroids, centroids_indices



def kmneas_pp():
    args = parse_arglist()
    k = args.k
    max_iter = args.max_iter
    epsilon = args.eps
    data_1 = pd.read_csv(args.file_name_1, header=None)
    data_2 = pd.read_csv(args.file_name_2, header=None)
    data = pd.merge(data_1, data_2, how='inner', on=0).set_index(0).sort_index().to_numpy()
    # data = np.array(data[data.columns[1:]])
    centroids, centroids_indices = select_centroids(data_points=data, n_centroids=k)

    result = fit(centroids.tolist(), data.tolist(), k, max_iter, epsilon)

    np.savetxt(sys.stdout.buffer, centroids_indices[None], fmt='%d', delimiter=',')
    np.savetxt(sys.stdout.buffer, result, fmt='%.4f', delimiter=',')


if __name__ == "__main__":
    kmneas_pp()

# parse_arglist()

