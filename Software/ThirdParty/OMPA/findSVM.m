% Notes:    Finds the standard vector magnitude of an input matrix.
% Input:    Takes in triaxial accelerometer data in row-wise fashion. 
%           E.g each row has an xaxis(col1), yaxis(col2), zaxis(col3)
% Output:   Standard vector mag (not removeing gravity component)

function svm=findSVM(xyz)
svm = sqrt(xyz(:,1).^2 + xyz(:,2).^2 + xyz(:,3).^2);
end