using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;

namespace OmGui
{
    public static class Utils
    {
        public static int BinarySearch<T>(this IList<T> list, int index, int length, T value, IComparer<T> comparer)
        {
            if (list == null) { throw new ArgumentNullException("list"); }
            else if (index < 0) { throw new ArgumentOutOfRangeException("index"); }
            else if (length < 0) { throw new ArgumentOutOfRangeException("length"); }
            else if (index + length > list.Count) { throw new ArgumentException(); }
            int lower = index;
            int upper = index + length - 1;
            while (lower <= upper)
            {
                int mid = lower + ((upper - lower) >> 1);
                int comparison = comparer.Compare(list[mid], value);
                if (comparison == 0) { return mid; }
                else if (comparison < 0) { lower = mid + 1; }
                else { upper = mid - 1; }
            }
            return ~lower;
        }

        public static int BinarySearch<T>(this IList<T> list, T value, IComparer<T> comparer)
        {
            return BinarySearch(list, 0, list.Count, value, comparer);
        }

        public static int BinarySearch<T>(this IList<T> list, T value) where T : IComparable<T>
        {
            return BinarySearch(list, value, Comparer<T>.Default);
        }

    }
}
