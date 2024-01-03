#include <stdio.h>
#include <stdlib.h>
#include <math.h>
//#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#include "stb_image_resize2.h"


// Data structure to represent RGB color
typedef struct RgbColor {
    unsigned char r, g, b;
} RgbColor;

// Data structure to represent a point in 3D space (RGB color space)
typedef struct Point {
    unsigned char r, g, b;
} Point;

// Data structure to represent a cluster
typedef struct Cluster {
    Point centroid;
    int count;
    Point* points;
} Cluster;

// Function to calculate Euclidean distance between two colors
double colorDistance(Point a, Point b) {
    double dr = a.r - b.r;
    double dg = a.g - b.g;
    double db = a.b - b.b;
    return sqrt(dr * dr + dg * dg + db * db);
}

// Function to initialize clusters with random points
void initializeClusters(Point* data, int dataCount, Cluster* clusters, int k) {
    for (int i = 0; i < k; i++) {
        int randIdx = rand() % dataCount;
        clusters[i].centroid = data[randIdx];
        clusters[i].count = 0;
        clusters[i].points = NULL;
    }
}

// Function to assign points to clusters
int assignPointsToClusters(Point* data, int dataCount, Cluster* clusters, int k) {
    int reassignments = 0;

    for (int i = 0; i < dataCount; i++) {
        Point p = data[i];
        int closestCluster = 0;
        double minDist = colorDistance(p, clusters[0].centroid);

        for (int j = 1; j < k; j++) {
            double dist = colorDistance(p, clusters[j].centroid);
            if (dist < minDist) {
                closestCluster = j;
                minDist = dist;
            }
        }

        if (clusters[closestCluster].points == NULL) {
            clusters[closestCluster].points = (Point*)malloc(sizeof(Point));
        } else {
            clusters[closestCluster].points = (Point*)realloc(clusters[closestCluster].points, (clusters[closestCluster].count + 1) * sizeof(Point));
        }

        clusters[closestCluster].points[clusters[closestCluster].count] = p;
        clusters[closestCluster].count++;

        if (i != closestCluster) {
            reassignments++;
        }
    }

    return reassignments;
}

// Function to update cluster centroids
// void updateClusterCentroids(Cluster* clusters, int k) {
//     for (int i = 0; i < k; i++) {
//         int sumR = 0, sumG = 0, sumB = 0;

//         for (int j = 0; j < clusters[i].count; j++) {
//             sumR += clusters[i].points[j].r;
//             sumG += clusters[i].points[j].g;
//             sumB += clusters[i].points[j].b;
//         }

//         clusters[i].centroid.r = (unsigned char)(sumR / clusters[i].count);
//         clusters[i].centroid.g = (unsigned char)(sumG / clusters[i].count);
//         clusters[i].centroid.b = (unsigned char)(sumB / clusters[i].count);
//     }
// }
void updateClusterCentroids(Cluster* clusters, int k) {
    for (int i = 0; i < k; i++) {
        int sumR = 0, sumG = 0, sumB = 0;

        if (clusters[i].count > 0) {
            for (int j = 0; j < clusters[i].count; j++) {
                sumR += clusters[i].points[j].r;
                sumG += clusters[i].points[j].g;
                sumB += clusters[i].points[j].b;
            }

            clusters[i].centroid.r = (unsigned char)(sumR / clusters[i].count);
            clusters[i].centroid.g = (unsigned char)(sumG / clusters[i].count);
            clusters[i].centroid.b = (unsigned char)(sumB / clusters[i].count);
        }
    }
}

int kMeans(Point* data, int dataCount, Cluster* clusters, int k) {
    int iterations = 0;
    int maxIterations = 100; // Add a maximum number of iterations to prevent infinite loops

    while (iterations < maxIterations) {
        int reassignments = 0; // Initialize reassignments to 0 for each iteration

        for (int i = 0; i < k; i++) {
            free(clusters[i].points);
            clusters[i].count = 0;
            clusters[i].points = NULL;
        }

        for (int i = 0; i < dataCount; i++) {
            Point p = data[i];
            int closestCluster = 0;
            double minDist = colorDistance(p, clusters[0].centroid);

            for (int j = 1; j < k; j++) {
                double dist = colorDistance(p, clusters[j].centroid);
                if (dist < minDist) {
                    closestCluster = j;
                    minDist = dist;
                }
            }

            if (clusters[closestCluster].points == NULL) {
                clusters[closestCluster].points = (Point*)malloc(sizeof(Point));
            } else {
                clusters[closestCluster].points = (Point*)realloc(clusters[closestCluster].points, (clusters[closestCluster].count + 1) * sizeof(Point));
            }

            clusters[closestCluster].points[clusters[closestCluster].count] = p;
            clusters[closestCluster].count++;

            if (i != closestCluster) {
                reassignments++;
            }
        }

        updateClusterCentroids(clusters, k);
        iterations++;

        // printf("Iteration %d: Reassignments = %d\n", iterations, reassignments); // Debug information

        if (reassignments == 0) {
            break; // If there are no reassignments, exit the loop early
        }
    }

    return iterations;
}


int domCol(const char* imagePath, RgbColor** domCols, int kNum) {
    // Load the image using stb_image
    int width, height, channels;
    unsigned char* image_data = stbi_load(imagePath, &width, &height, &channels, 3);
    // TODO: scale down
    if (image_data==NULL) {
        printf("Failed to load image.\n");
        return 1;
    }
    if(width > 100 || height > 100){
        printf("Resizing image.\n");
        int new_width, new_height;
        if (width > height) {
            new_width = 100;
            new_height = (int)((float)height * (100.0f / width));
        } else {
            new_width = (int)((float)width * (100.0f / height));
            new_height = 100;
        }

        // Create a new buffer for the scaled-down image
        unsigned char* scaled_image_data = (unsigned char*)malloc(new_width * new_height * channels);

        // Use stbir library to scale the image
        stbir_resize_uint8_linear(image_data, width, height, 0, scaled_image_data, new_width, new_height, 0, channels);
        free(image_data);
        width = new_width;
        height = new_height;
        image_data = scaled_image_data;
    }

    int k = kNum; // Number of clusters (dominant colors)

    int dataCount = width * height;

    // Initialize data points from the image
    Point* data = (Point*)malloc(dataCount * sizeof(Point));
    for (int i = 0; i < dataCount; i++) {
        data[i].r = image_data[i * 3];
        data[i].g = image_data[i * 3 + 1];
        data[i].b = image_data[i * 3 + 2];
    }

    // Initialize clusters
    Cluster clusters[k];
    initializeClusters(data, dataCount, clusters, k);

    int iterations = kMeans(data, dataCount, clusters, k);

    *domCols = (RgbColor*) calloc(k, sizeof(RgbColor));
    // Display the dominant colors
    printf("Dominant colors after %d iterations:\n", iterations);
    for (int i = 0; i < k; i++) {
        // printf("RgbColor %d (RGB): (%d, %d, %d)\n", i + 1, clusters[i].centroid.r, clusters[i].centroid.g, clusters[i].centroid.b);
        // (*domCols)[i] = clusters[i].centroid;
        (*domCols)[i].r = clusters[i].centroid.r;
        (*domCols)[i].g = clusters[i].centroid.g;
        (*domCols)[i].b = clusters[i].centroid.b;
    }

    // Free memory
    stbi_image_free(image_data);
    free(data);
    for (int i = 0; i < k; i++) {
        free(clusters[i].points);
    }

    return 0;
}