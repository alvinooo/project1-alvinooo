import sys

concurrent_requests = int(sys.argv[1])

latencies = []
for i in xrange(concurrent_requests):
	with open("Latency%i.txt" % i) as output:
		string = output.read()
		start = string.find("time= ")
		end = string.find(" ms")
		latencies.append(float(string[start+6:end]))

print "Latencies (ms)"
print latencies
print "Average latency (ms)"
print sum(latencies) / len(latencies)

latencies = []
for i in xrange(concurrent_requests):
	with open("Latency%i.txt" % i) as output:
		string = output.read()
		start = string.find("time= ")
		end = string.find(" ms")
		latencies.append(float(string[start+6:end]))

print "Latencies (ms)"
print latencies
print "Average latency (ms)"
print sum(latencies) / len(latencies)