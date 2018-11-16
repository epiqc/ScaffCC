import warnings
warnings.filterwarnings("ignore", message="numpy.dtype size changed")
# warnings.filterwarnings("ignore", message="numpy.ufunc size changed")
#
# from scipy.stats import chisquare
from scipy import stats

import numpy as np

# a1 = [1/8, 0.001, 1/8, 0.001, 1/8, 0.001, 1/8, 0.001]
# a2 = [1/64, 1/64, 1/64, 1/64, 1/64, 1/64, 1/64, 1/64]
# a3 = [1/64, 1/64, 1/64, 1/64, 1/64, 1/64, 1/64, 1/64]
# a4 = [1/64, 1/64, 1/64, 1/64, 1/64, 1/64, 1/64, 1/64]
# a5 = [1/64, 1/64, 1/64, 1/64, 1/64, 1/64, 1/64, 1/64]

# a1 = [8, 8, 8, 8]
# a2 = [8, 8, 8, 8]
# a3 = [8, 8, 8, 8]
# a4 = [8, 8, 8, 8]
# a5 = [8, 8, 8, 8]
# a1 = [8, 0, 8, 0, 8, 0, 8, 0]
# a2 = [1, 1, 1, 1, 1, 1, 1, 1]
# a3 = [1, 1, 1, 1, 1, 1, 1, 1]
# a4 = [1, 1, 1, 1, 1, 1, 1, 1]
# a5 = [1, 1, 1, 1, 1, 1, 1, 1]

# a1 = [8,1,1,1,1]
# a2 = [0,1,1,1,1]
# a3 = [8,1,1,1,1]
# a4 = [0,1,1,1,1]
# a5 = [8,1,1,1,1]
# a6 = [0,1,1,1,1]
# a7 = [8,1,1,1,1]
# a8 = [0,1,1,1,1]


# shor = 32*np.array([a2, a3, a4, a5])
# shor = 32*np.array([a1, a2, a3, a4, a5])
# shor = np.array([a1, a2, a3, a4, a5, a6, a7, a8])
# shor = np.transpose(np.array([a1, a2, a3, a4, a5]))


#
print stats.chisquare([1,6])
# chisquare([16, 18, 16, 14, 12, 12], f_exp=[16, 16, 16, 16, 16, 8])
#
# chi2_stat, p_val, dof, ex = stats.chi2_contingency(shor)
#
# print("===Chi2 Stat===")
# print(chi2_stat)
# print("\n")
#
# print("===Degrees of Freedom===")
# print(dof)
# print("\n")
#
# print("===P-Value===")
# print(p_val)
# print("\n")
#
# print("===Contingency Table===")
# print(ex)
# scipy.stats.entropy(pk, qk=None, base=None)[source]

print "done"
