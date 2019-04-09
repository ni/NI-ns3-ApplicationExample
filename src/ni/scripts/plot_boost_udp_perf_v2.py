#!/usr/bin/env python

import sys
import csv
import numpy as np
import pylab
import pdb
import matplotlib.pyplot as plt


if __name__ == "__main__":
    csvPath = sys.argv[1]

    # choose between "latency" and "jitter" for displaying either the TX jitter plot or the latency histogram
    plotOpt = sys.argv[2]
    
    txTs     = list()
    rxTs     = list()
    latency  = list()
    idx_lst  = list()
    absTxTs  = list()
    
    scale = 1000.0
    count = 0
    
    first_row = True
    init_idx = 0
    
    skip_init_rows = 10

    # add variable for packet loss
    packetsLost = 0

    numBytes = 0
    
    with open(csvPath, 'rb') as csvFile:
        reader = csv.reader(csvFile, delimiter=' ')
        try:
            # skip first skip_init_rows rows
            for row in reader:
                if skip_init_rows > 0:
                    skip_init_rows -= 1
                    continue

                if(len(row) == 6):
                    idx = int(row[0])

                    if first_row:
                        first_row = False
                        init_idx = idx
                        idx=idx-init_idx
                        idx_lst.append(idx)
                        txTs.append(float(row[1]))
                        rxTs.append(float(row[2]))
                        latency.append(float(row[3]))
                        absTxTs.append(float(row[4]))
                        continue

                    idx=idx-init_idx

                    if first_row == False:
                        idx_lst.append(idx)
                        txTs.append(float(row[1]))
                        rxTs.append(float(row[2]))
                        latency.append(float(row[3]))
                        absTxTs.append(float(row[4]))

                    # increase packetLost counter every time there is a zero line
                    if (int(row[0]) == 0):
                        packetsLost += 1
                    else:
                        numBytes = int(row[5])

        except csv.Error as e:
            sys.exit('file %s, line %d: %s' % (csvPath, reader.line_num, e))

    txTs = np.array(txTs)
    #txTs = txTs[np.where(txTs>0)]   # sort out all zero lines
    rxTs = np.array(rxTs)
    #rxTs = rxTs[np.where(rxTs>0)]
    
    latency = np.array(latency) / scale
    latency = latency[np.where(latency>0)]

    # calculate the relative loss
    packetLoss = float(packetsLost) / len(idx_lst)

    txJitter = np.zeros(len(txTs))
    rxJitter = np.zeros(len(rxTs))

    prevLineZero = False

    for idx in xrange(len(txTs)):
        # skip first zero line
        if (txTs[idx] == 0):
            prevLineZero = True
            continue

        # skip next full line after a zero line to avoid miscalculations of txJitter
        if (prevLineZero == True):
            prevLineZero = False
            continue

        if (prevLineZero == False):
            if(idx > 0):
                if(txTs[idx] > txTs[idx-1] and txTs[idx-1] != 0):
                    txJitter[idx] = (txTs[idx] - txTs[idx-1]) / scale
                else:
                    txJitter[idx] = (1000000000 - txTs[idx-1] + txTs[idx]) / scale
                if (txJitter[idx] < 0): txJitter[idx] = 0
            count += 1


    # remove zero lines out of txJitter array
    txJitter = txJitter[np.where(txJitter > 0)]

    absTxTs = np.array(absTxTs)

    # remove zero lines for correct throughput calculation
    absTxTs = absTxTs[np.where(absTxTs>0)]

    # total runtime (first packet sent - last packet received)
    runTimeUs = (float) (absTxTs[-1] - absTxTs[0]) + latency[-1] / 1000

    # throughput as quotient of number of successfully transmitted bits and total runtime
    throughput = (float)(len(absTxTs)*8*numBytes) / runTimeUs

    txJitterMax = np.max(txJitter[1:(count-1)])
    txJitterMin = np.min(txJitter[1:(count-1)])
    txJitterMean = np.mean(txJitter[1:(count-1)])
    txJitterStd = np.std(txJitter[1:(count-1)])

    rxJitterMax = np.max(rxJitter[1:(count-1)])
    rxJitterMin = np.min(rxJitter[1:(count-1)])
    rxJitterMean = np.mean(rxJitter[1:(count-1)])
    rxJitterStd = np.std(rxJitter[1:(count-1)])
    
    latencyMax = np.max(latency[0:(count-1)])
    latencyMin = np.min(latency[0:(count-1)])
    latencyMean = np.mean(latency[0:(count-1)])
    latencyStd = np.std(latency[0:(count-1)])

    txJitterLabel = "TxJitter Stats: maximum = %.2f us, minimum = %.2f us, mean = %.2f us, standard deviation = %.2f us" % (txJitterMax, txJitterMin, txJitterMean, txJitterStd)
    print txJitterLabel

    latencyLabel = "Latency Stats: maximum = %.2f us, minimum = %.2f us, mean = %.2f us, standard deviation = %.2f us" % (latencyMax, latencyMin, latencyMean, latencyStd)
    print latencyLabel

    packetLossLabel = "Packet Loss Stats: number of packets lost: %d, relative packet loss: %.3f %%" % (packetsLost, packetLoss*100)
    print packetLossLabel

    print "Total Runtime: %.3f s" % (runTimeUs/1000000)

    print "Throughput: %.3f Mbit/s" % throughput

    plt.close('all')

    from matplotlib2tikz import save as tikz_save

    if plotOpt == "latency":
        #plt.figure(figsize=(18, 5))
        latencyHist = plt.hist(latency, bins=200)
        plt.tick_params(axis='x')
        plt.tick_params(axis='y')
        plt.title("end-to-end latency histogram:\n maximum = $%.2f \mu s$, minimum = $%.2f \mu s$,\n mean = $%.2f \mu s$, standard deviation = $%.2f \mu s$" % (latencyMax, latencyMin, latencyMean, latencyStd), fontsize=16)
        plt.xlabel(r"latency in $\mu s$", fontsize=16)
        plt.ylabel("number of packets", fontsize=16)
        plt.savefig("latency.eps", format='eps', bbox_inches='tight')

        #np.savetxt("latency.txt", latencyHist, fmt='%5s', delimiter=",")
        #tikz_save('latency.tex')

    if plotOpt == "jitter":
        plt.figure(figsize=(18, 5))
        plt.plot(txJitter[1:(count-1)])
        plt.tick_params(axis='x')
        plt.tick_params(axis='y')
        plt.title("TX event jitter:\n maximum = $%.2f \mu s$, minimum = $%.2f \mu s$,\n mean = $%.2f \mu s$, standard deviation = $%.2f \mu s$" % (txJitterMax, txJitterMin, txJitterMean, txJitterStd), fontsize=16)
        plt.xlabel("packet index", fontsize=14)
        plt.ylabel(r"time between TX events in $\mu s$", fontsize=14)
        plt.xlim(0, 10000)
        plt.ylim(txJitterMax-200, txJitterMax+200)
        plt.savefig("txJitter.eps", format='eps', bbox_inches='tight')

        #np.savetxt("txJitter.txt", txJitter, delimiter=",")
        #tikz_save('txJitter.tex')

    '''
    #f, axarr = plt.subplots(2, sharex=True)
    f, axarr = plt.subplots(3)

    axarr[0].plot(txJitter[1:(count-1)], color='green')
    axarr[0].set_title('UDP perf test (Boost): TX event jitter (us resolution)')
    #axarr[0].set_text(0, 0, txJitterLabel, fontsize=12)
    axarr[0].set_xlabel("packet index")
    axarr[0].set_ylabel("time between TX events in us")
    axarr[0].set_ylim(txJitterMax-100, txJitterMax+100)

    axarr[1].plot(latency[0:(count-1)])
    axarr[1].set_title('UDP perf test (Boost): end-to-end latency (in usec resolution)')
    #axarr[1].set_text(0, 0, latencyLabel, fontsize=12)
    axarr[1].set_xlabel("packet index")
    axarr[1].set_ylabel("latency (us)")

    #axarr[2].plot(rxJitter[1:(count-1)])
    #axarr[2].set_title('UDP perf test (Boost): RX event jitter (us resolution)')
    #axarr[2].set_text(0, 0, txJitterLabel, fontsize=12)
    #axarr[2].set_xlabel("packet index")
    #axarr[2].set_ylabel("time between RX events in us")

    plt.hist(latency, bins=200)
    plt.title("UDP perf test (Boost): end-to-end latency histogram")
    plt.xlabel("latency (us)")
    plt.ylabel("number of packets")
    '''

    plt.show()