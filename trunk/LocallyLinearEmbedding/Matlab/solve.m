% Setup cvx
addpath('cvx-rd/cvx')
cvx_setup

%cvx_solver sedumi

% Read in the matrices
b = dlmread('b.txt');
Qraw = dlmread('Q.txt');
Q = spconvert(Qraw);

n = size(b,1);

if size(Q,1) ~= n || size(Q,2) ~= n
    Q(n,n) = 0;
end

% Specify the problem
cvx_begin
    variable x(n)
    minimize( norm(Q*x-b,2) )
    subject to
        x >= 0
        x <= 1
cvx_end

%x = lsqlin(Q,b,[],[],[],[],0,1)
%x = lsqlin(Q,b,[],[],[],[],0,[]);

%save x to a file
dlmwrite('x.txt', x);


