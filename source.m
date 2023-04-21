clear all;
clc;

% some functions to remember
% imread, rgb2gray, imcomplement, imbinarize, bwmorph, imcomplement,
% imshow, ocr, imresize

templates = zeros(23, 53, 36);
chars = ["1" "2" "3" "4" "5" "6" "7" "8" "9" "0" "A" "B" "E" "H" "K" "M" "N" "P" "T" "X" "Y" "Z"];
for i=1:size(chars, 2)
    im0 = imread("/home/vasilis/Multimedia/Pictures/" + chars(i) + ".png");
    im0 = imresize(im0, [53 36]);
    im0 = rgb2gray(im0);
    if i == 4
        im0 = imcomplement(im0);
    end
    im0 = imbinarize(im0);
    templates(i, :, :) = im0;
    % imshow(im0);
end

image = imread('/home/vasilis/Multimedia/Pictures/car2.jpg');
imshow(image);

% 2ND FUNCTION
candidatesFileID = fopen('/home/vasilis/Programming/C++ Projects/LPR/candidates2', 'r');
quadrilaterals = fscanf(candidatesFileID, '%d');
q = quadrilaterals;

imshow(image);
hold on;
for i=1:size(quadrilaterals, 1)/8-1
    plot([q(2+i*8), q(4+i*8), q(8+i*8), q(6+i*8), q(2+i*8)], [q(1+i*8), q(3+i*8), q(7+i*8), q(5+i*8), q(1+i*8)], 'LineWidth', 2);
end

prevs = "";

for i=29:size(quadrilaterals, 1)/8-1
    % plot([q(2+i*8), q(4+i*8), q(8+i*8), q(6+i*8), q(2+i*8)], [q(1+i*8), q(3+i*8), q(7+i*8), q(5+i*8), q(1+i*8)], 'LineWidth', 2);
    imshow(image(min([q(1+i*8), q(3+i*8)])+1:max([q(7+i*8), q(5+i*8)])+1, min([q(2+i*8), q(4+i*8)])+1:max([q(8+i*8), q(6+i*8)])+1, :));
    candidate = image(min([q(1+i*8), q(3+i*8)])+1:max([q(7+i*8), q(5+i*8)])+1, min([q(2+i*8), q(4+i*8)])+1:max([q(8+i*8), q(6+i*8)])+1, :);
    candidate = rgb2gray(candidate);
    candidate = imbinarize(candidate);
    candidate = imcomplement(candidate);
    imshow(candidate);
    cc = bwlabel(candidate);
    
    labelsFiltered = [];
    boundaries = [];
    for k=1:max(max(cc))
        [r, c] = find(cc==k);
        toplefty = 10000;
        topleftx = 10000;
        bottomrighty = 0;
        bottomrightx = 0;
        for f=1:size(r, 1)
            if(r(f) < toplefty)
                toplefty = r(f);
            end
            if(r(f) > bottomrighty)
                bottomrighty = r(f);
            end
            if(c(f) < topleftx)
                topleftx = c(f);
            end
            if(c(f) > bottomrightx)
                bottomrightx = c(f);
            end
        end
        if 1.3 <= (bottomrighty - toplefty)/(bottomrightx - topleftx) && (bottomrighty - toplefty)/(bottomrightx - topleftx) <= 3
            if(bottomrighty - toplefty > 3*(q(5+i*8) - q(1+i*8))/5)
                labelsFiltered = [labelsFiltered k];
                boundaries = [boundaries; toplefty topleftx bottomrighty bottomrightx];
            end
        end
    end
    
    im = zeros(size(candidate));
    for k=labelsFiltered
        [r, c] = find(cc==k);
        
        for f=1:size(r, 1)
            im(r(f), c(f)) = 1;
        end
    end
    imshow(im);
    
    H = zeros(2, 2);
    H(:, 1) = [1; 0];
    xh = q(2+i*8) + (q(5+i*8) - q(1+i*8))*4.75;
    yh = q(1+i*8);
    H(:, 2) = [(yh - q(3+i*8))/q(4+i*8); xh/q(4+i*8)];
    if size(labelsFiltered, 2) == 7
        fad = "";
        for d = 1 : 7
            char = candidate(boundaries(d, 1):boundaries(d, 3), boundaries(d, 2):boundaries(d, 4));
            charyo = zeros(53, 50, 'logical');
            for bv=1:size(char, 1)
                for bn=1:size(char, 2)
                    if char(bv, bn) == 1
                        al = H*[bv;bn];
                        al = round(al);
                        if(al(1) <= 0) al(1) = 1; 
                        end
                        if(al(2) <= 0) al(2) = 1; 
                        end
                        charyo(uint32(round(al(1))), uint32(round(al(2)))) = 1;
                    end
                end
            end
            char = charyo;
            char = imresize(char, [53 36]);
            imshow(char);
            bestv = 1;
            bestcorrcoef = 0;
            reals = [];
            for gh=1:53
                reals = [reals char(gh, :)];
            end
            for v=1:23
                templo = [];
                for x=1:53
                    for b = 1:36
                        templo = [templo templates(v, x, b)];
                    end
                end
                R = corrcoef(reals', templo');
                if R(1, 2) > bestcorrcoef
                    bestcorrcoef = R(1, 2);
                    bestv = v;
                end
            end
        fad = fad + chars(bestv);    
        end
        if prevs ~= fad
            prevs = fad;
            fprintf("%s", fad);
        end
    end
end

% 1ST FUNCTION
% mask = [-1 -2 -3 -4 -5 0 5 4 3 2 1; -1 -2 -3 -4 -5 0 5 4 3 2 1; -1 -2 -3 -4 -5 0 5 4 3 2 1];
% image = rgb2gray(image);
% Gmag = conv2(image, mask);
% Gmag = abs(Gmag);
% Gmag = (Gmag - (min(min(Gmag))))/max(max(Gmag));
% Gmag = Gmag(2:size(Gmag, 1)-1, 6:size(Gmag, 2)-5);
% outputImage = imbinarize(Gmag); 
% imshow(outputImage);
% dlmwrite('/home/vasilis/Programming/C++ Projects/LPR/edge10', outputImage, 'delimiter', ' ');