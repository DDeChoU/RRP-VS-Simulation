import heapq
class A:
	def __init__(self,a):
		self.same = 1
		self.a = a
	def __gt__(self, aa):
		return self.a>aa.a

a_list = []
heapq.heapify(a_list)
a1 = A(1)
a2 = A(2)
a5 = A(5)
heapq.heappush(a_list, a1)
heapq.heappush(a_list, a2)
heapq.heappush(a_list, a5)
for i in range(len(a_list)):
	print(heapq.heappop(a_list).a)
