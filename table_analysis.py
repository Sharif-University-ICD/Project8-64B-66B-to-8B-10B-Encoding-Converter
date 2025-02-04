import lookup_table_810


def max_consecutive_bits(number, bit='0'):
    binary_rep = bin(number)[1:]
    target = '0' if bit == '0' else '1'
    max_consecutive = max((len(segment) for segment in binary_rep.split('1' if bit == '0' else '0')), default=0)
    return max_consecutive


def count_transitions(number):
    binary_rep = bin(number)[1:]
    zero_to_one = sum(1 for i in range(1, len(binary_rep)) if binary_rep[i - 1] == '0' and binary_rep[i] == '1')
    one_to_zero = sum(1 for i in range(1, len(binary_rep)) if binary_rep[i - 1] == '1' and binary_rep[i] == '0')
    return zero_to_one, one_to_zero


def max_difference_ones_zeros(number):
    binary_rep = bin(number)[:]
    count_ones = binary_rep.count('1')
    count_zeros = binary_rep.count('0')
    return abs(count_ones - count_zeros)


def run_tests():
    table = lookup_table_810.lookup_8B_10B
    max_zeros = 0
    max_ones = 0
    max_zero_to_one = 0
    max_one_to_zero = 0
    max_diff_ones_zeros = 0

    for number in table:
        consecutive_zeros = max_consecutive_bits(number, bit='0')
        consecutive_ones = max_consecutive_bits(number, bit='1')
        zero_to_one, one_to_zero = count_transitions(number)
        difference_ones_zeros = max_difference_ones_zeros(number)

        max_zeros = max(max_zeros, consecutive_zeros)
        max_ones = max(max_ones, consecutive_ones)
        max_zero_to_one = max(max_zero_to_one, zero_to_one)
        max_one_to_zero = max(max_one_to_zero, one_to_zero)
        max_diff_ones_zeros = max(max_diff_ones_zeros, difference_ones_zeros)

    print(f"Maximum number of consecutive zeros: {max_zeros}")
    print(f"Maximum number of consecutive ones: {max_ones}")
    print(f"Maximum number of 0-to-1 transitions: {max_zero_to_one}")
    print(f"Maximum number of 1-to-0 transitions: {max_one_to_zero}")
    print(f"Maximum difference between number of ones and zeros: {max_diff_ones_zeros}")


if __name__ == "__main__":
    run_tests()