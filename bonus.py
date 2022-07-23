import numpy as np
import matplotlib.pyplot as plt
from sklearn.cluster import KMeans
from sklearn import datasets

iris = datasets.load_iris()

inertias = [KMeans(n_clusters=k, random_state=0).fit(iris.data).inertia_ for k in range(1, 11)]

plt.title("Elbow Method for selection of optimal 'K' clusters")
plt.xlabel("K")
plt.ylabel('Inertia')
plt.plot(np.arange(1, 11), inertias)
plt.xticks(range(1, 11))
ax = plt.gca()
y_len = ax.get_ylim()[1] - ax.get_ylim()[0]
x_len = ax.get_xlim()[1] - ax.get_xlim()[0]
plt.scatter(3, inertias[2], 400, facecolors='none', edgecolors='k')

plt.savefig('bonus.png')
