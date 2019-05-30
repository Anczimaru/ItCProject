function [OUT] = modInv(a,m)
%UNTITLED2 Summary of this function goes here
%   Detailed explanation goes here
[g,c, ~] = gcd(a,m);
if g == 1
   OUT = mod(c,m)
else
    disp("CHUJ")
end
end

