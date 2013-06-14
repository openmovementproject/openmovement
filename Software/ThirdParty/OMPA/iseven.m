function ise = iseven(x)
%ISEVEN True for even numbers.
%   ISEVEN(X) is 1 for the elements of X that are even and 0 for odd elements.
%
%   Class support for input X:
%      float: double, single
%      integer: [u]int8, [u]int16, [u]int32, [u]int64
%
%   An error is raised if X is a float and is:
%   1) too large or small to determine if it is odd or even (this includes -Inf
%      and +Inf), or
%   2) contains a fraction (e.g. isodd(1.5) raises an error), or
%   3) NaN (not-a-number).
%
%   See also ISODD.

% By Ulf Carlberg 29-MAY-2011.

ise = ~isodd(x);