import matplotlib.pyplot as plt
# plt.style.use('ggplot')

with open("../samples/log.txt", "r") as fp:
    lines = fp.readlines()

queue = []
avg=[]
for count, line in enumerate(lines):
    if count == 0:
        traffic = line
    else:
        queue_len, avg_len = line.split("\t")
        queue.append(float(queue_len))
        avg.append(float(avg_len))

plt.figure(num=None, figsize=(12, 7), dpi=90, facecolor='w', edgecolor='k')
plt.locator_params(axis='x', nbins=10)
plt.plot(range(len(queue)), queue, color='#32CD32', marker='+', label="Current queue length")
plt.plot(range(len(avg)), avg, color='red', marker='D', ms=3, label="Average queue length")
plt.legend()

# plt.show()
fileName = "./samples/" + traffic.strip() + "/queues.png"
plt.savefig(fileName, bbox_inches='tight')
print("Graph plotted successfully")
