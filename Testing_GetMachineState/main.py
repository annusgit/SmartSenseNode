import matplotlib.pyplot as plt
import csv

n_for_rms_status_assignment = 50
state_change_criteria = (0.9 * n_for_rms_status_assignment)
off_current_threshold = 0.5
MACHINE_STATES = {0: 'MACHINE_OFF', 1: 'MACHINE_IDLE', 2: 'MACHINE_ON', 3: 'SENSOR_NOT_CONNECTED'}
idle_current_threshold = 5
RMS_long_buffer = list()


# def readandstorecurrentsfromcsvfile():
#     f=open('Tumbler_1_230920.csv')
#     csv_f=csv.reader(f)
#     loadcurrents=[]
#     for row in csv_f:
#         loadcurrents.append(row[10])
#         print (row[10])
#
#     for k in range(rms_status_assignment_sample_count):
#         RMS_long_buffer[machine_no*n_for_rms_status_assignment + rms_status_assignment_sample_count] = loadcurrents(k)
#     pass


def getmachinestatus():
    global RMS_long_buffer, state_change_criteria, off_current_threshold, idle_current_threshold, n_for_rms_status_assignment
    OFF_count, ON_count, IDLE_count = 0, 0, 0
    for j in range(n_for_rms_status_assignment):
        if 0 <= RMS_long_buffer[j] <= off_current_threshold:
            OFF_count += 1
        elif off_current_threshold <= RMS_long_buffer[j] <= idle_current_threshold:
            IDLE_count += 1
        else:
            ON_count += 1
            pass
    if OFF_count >= state_change_criteria:
        return 0
    elif IDLE_count >= state_change_criteria:
        return 1
    elif ON_count >= state_change_criteria:
        return 2
    else:
        if OFF_count > ON_count and IDLE_count > ON_count:
            return 0
        elif ON_count > OFF_count and IDLE_count > OFF_count:
            return 2
        return -1  # this indicates that we don't need to change our state


def main():
    global RMS_long_buffer, n_for_rms_status_assignment
    for i in range(n_for_rms_status_assignment):
        RMS_long_buffer.append(0)
        pass
    rms_status_assignment_sample_count = 0
    new_states, prev_state = [], 3
    load_currents = []
    with open('Tumbler_1_230920.csv', 'r') as data_file:
        for index, row in enumerate(data_file):
            if index % 1000 == 0:
                print(f"LOG: On Index {index}")
            if not row or index == 0:
                continue
            try:
                current_value = float(str(row.strip().split(',')[-1]))
            except ValueError:
                print(row.strip().split(',')[-1])
                pass
            load_currents.append(current_value)
            RMS_long_buffer[rms_status_assignment_sample_count] = current_value
            rms_status_assignment_sample_count += 1
            if rms_status_assignment_sample_count >= n_for_rms_status_assignment:
                rms_status_assignment_sample_count = 0
                pass
            updated_status = getmachinestatus()
            if updated_status == -1:
                new_states.append(prev_state)
            else:
                new_states.append(updated_status)
                prev_state = updated_status
        pass
    pass
    fig, axs = plt.subplots(2)
    fig.suptitle('State Assignment Test')
    axs[0].plot(load_currents[:1000])
    axs[1].plot(new_states[:1000])
    axs[0].set_xlabel('Load Currents-Irms')
    axs[0].set_ylabel('Machine States')
    axs[1].set_ylabel('Machine States')
    axs[1].set_xlabel('Load Currents-Irms')

    plt.show()
pass


if __name__ == '__main__':
    main()
    pass


