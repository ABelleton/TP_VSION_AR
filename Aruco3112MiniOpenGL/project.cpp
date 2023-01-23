//
//  ArUco-OpenGL.cpp
//
//  Created by Jean-Marie Normand on 28/02/13.
//  Copyright (c) 2013 Centrale Nantes. All rights reserved.
//


#include "project.h"
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2\calib3d.hpp>

#include <ctime>

float FixColorR;
float FixColorG;
float FixColorB;

float sauvColorR;
float sauvColorG;
float sauvColorB;

void initColor() {
    float FixColorR = 0.0;
    float FixColorG = 0.0;
    float FixColorB = 0.0;

    float sauvColorR = 0.0;
    float sauvColorG = 0.0;
    float sauvColorB = 0.0;
}

void HSVtoRGB(float H, float S, float V, float &sauvColorR, float &sauvColorG, float &sauvColorB) {
    if (H > 360 || H < 0 || S>100 || S < 0 || V>100 || V < 0) {
        std::cout << "The givem HSV values are not in valid range" << std::endl;
        return;
    }
    float s = S / 100;
    float v = V / 100;
    float C = s * v;
    float X = C * (1 - abs(fmod(H / 60.0, 2) - 1));
    float m = v - C;
    float r, g, b;
    if (H >= 0 && H < 60) {
        r = C, g = X, b = 0;
    }
    else if (H >= 60 && H < 120) {
        r = X, g = C, b = 0;
    }
    else if (H >= 120 && H < 180) {
        r = 0, g = C, b = X;
    }
    else if (H >= 180 && H < 240) {
        r = 0, g = X, b = C;
    }
    else if (H >= 240 && H < 300) {
        r = X, g = 0, b = C;
    }
    else {
        r = C, g = 0, b = X;
    }
    sauvColorR = (r + m);
    sauvColorG = (g + m);
    sauvColorB = (b + m);
    std::cout << "rouge : " << sauvColorR << " - vert : " << sauvColorG << " - bleu : " << sauvColorB << endl;
}

// Constructor
ArUco::ArUco(string intrinFileName, float markerSize) {
    // Initializing attributes
    m_IntrinsicFile = intrinFileName;
    m_MarkerSize = markerSize;
    // read camera parameters if passed
    m_CameraParams.readFromXMLFile(intrinFileName);

}

// Destructor
ArUco::~ArUco() {}

void ArUco::resizeCameraParams(cv::Size newSize) {
    m_CameraParams.resize(newSize);
}

// Detect marker and draw things
void ArUco::doWork(Mat inputImg) {
    m_InputImage = inputImg;
    m_GlWindowSize = m_InputImage.size();
    m_CameraParams.resize(m_InputImage.size());
    resize(m_GlWindowSize.width, m_GlWindowSize.height);
}

// Draw axis function
void ArUco::drawAxis(float axisSize) {
    // X
    glColor3f(1, 0, 0);
    glBegin(GL_LINES);
    glVertex3f(0.0f, 0.0f, 0.0f); // origin of the line
    glVertex3f(axisSize, 0.0f, 0.0f); // ending point of the line
    glEnd();

    // Y
    glColor3f(0, 1, 0);
    glBegin(GL_LINES);
    glVertex3f(0.0f, 0.0f, 0.0f); // origin of the line
    glVertex3f(0.0f, axisSize, 0.0f); // ending point of the line
    glEnd();

    // Z
    glColor3f(0, 0, 1);
    glBegin(GL_LINES);
    glVertex3f(0.0f, 0.0f, 0.0f); // origin of the line
    glVertex3f(0.0f, 0.0f, axisSize); // ending point of the line
    glEnd();
}

// Fonction qui importe un objet
void drawForm(GLfloat size, GLenum type) {
    Model_OBJ form = Model_OBJ();
    form.Load("../computer.obj");
    form.Draw();
    form.Release();
}

// Fonction qui dessine un plan coloré
void drawPlane(GLfloat size, GLenum type)
{
    static const GLfloat n[1][3] =
    {
      {0.0, 0.0, 1.0}
    };
    static const GLint faces[1][4] =
    {
      {0, 1, 2, 3}
    };
    GLfloat v[4][3];

    v[0][2] = v[1][2] = v[2][2] = v[3][2] = -size / 2;
    v[0][1] = v[1][1] = -size / 2;
    v[2][1] = v[3][1] = size / 2;
    v[0][0] = v[3][0] = -size / 2;
    v[1][0] = v[2][0] = size / 2;

    glBegin(GL_QUADS);
    glNormal3fv(&n[0][0]);
    glVertex3fv(&v[faces[0][0]][0]);
    glVertex3fv(&v[faces[0][1]][0]);
    glVertex3fv(&v[faces[0][2]][0]);
    glVertex3fv(&v[faces[0][3]][0]);
    glEnd();
}


void checkDistance(float sauvX, float sauvY, float sauvR, Marker m2) {
    cout << " x1 : " << sauvX << " | y1 : " << sauvY << endl;
    cout << " x2 : " << m2.getCenter().x << " | y2 : " << m2.getCenter().y << endl;
    cout << " r1 : " << sauvR << " | r2 : " << m2.getRadius() << endl;
    if (pow(sauvX - m2.getCenter().x, 2) + pow(sauvY - m2.getCenter().y, 2) < pow(sauvR + m2.getRadius(), 2)) {
        FixColorR = sauvColorR;
        FixColorG = sauvColorG;
        FixColorB = sauvColorB;
    }
}

void addForm(GLdouble size) {
    drawForm(size, GL_LINE_LOOP);
}

void drawChangingPlane(GLdouble size) {
    drawPlane(size, GL_LINE_LOOP);
}

// GLUT functionnalities

// Drawing function
void ArUco::drawScene(double duration) {
    // If we do not have an image we don't do anyhting
    if (m_ResizedImage.rows == 0)
        return;

    float sauvX = 10000;
    float sauvY = 10000;
    float sauvR = 0;

    // On "reset" les matrices OpenGL de ModelView et de Projection
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();

    // On deinit une vue orthographique de la taille de l'image OpenGL
    glOrtho(0, m_GlWindowSize.width, 0, m_GlWindowSize.height, -1.0, 1.0);
    // on definit le viewport correspond a un rendu "plein ecran"
    glViewport(0, 0, m_GlWindowSize.width, m_GlWindowSize.height);

    // on desactive les textures
    glDisable(GL_TEXTURE_2D);

    // On "flippe" l'axe des Y car OpenCV et OpenGL on un axe Y inverse pour les images/textures
    glPixelZoom(1, -1);

    // On definit la position ou l'on va ecrire dans l'image
    glRasterPos3f(0, m_GlWindowSize.height, -1.0f);

    // On "dessine" les pixels contenus dans l'image OpenCV m_ResizedImage (donc l'image de la Webcam qui nous sert de fond)
    glDrawPixels(m_GlWindowSize.width, m_GlWindowSize.height, GL_RGB, GL_UNSIGNED_BYTE, m_ResizedImage.ptr(0));

    // On active ensuite le depth test pour les objets 3D
    glEnable(GL_DEPTH_TEST);

    // On passe en mode projection pour definir la bonne projection calculee par ArUco
    glMatrixMode(GL_PROJECTION);
    double proj_matrix[16];
    m_CameraParams.glGetProjectionMatrix(m_ResizedImage.size(), m_GlWindowSize, proj_matrix, 0.01, 100);
    glLoadIdentity();
    // on charge la matrice d'ArUco 
    glLoadMatrixd(proj_matrix);

    // On affiche le nombre de marqueurs (ne sert a rien)
    double modelview_matrix[16];
    std::cout << "Number of markers: " << m_Markers.size() << std::endl;

    // On desactive le depth test
    //glDisable(GL_DEPTH_TEST);
    glClear(GL_DEPTH_BUFFER_BIT);

    // Pour chaque marqueur detecte
    for (unsigned int m = 0; m < m_Markers.size(); m++)
    {
        if (m_Markers[m].id == 143 || m_Markers[m].id == 186) {
            // On recupere la matrice de modelview qui correspond au marqueur [m]
            m_Markers[m].glGetModelViewMatrix(modelview_matrix);
            glMatrixMode(GL_MODELVIEW);
            glLoadIdentity();
            // On charge cette matrice pour se placer dans le repere de ce marqueur [m] 
            glLoadMatrixd(modelview_matrix);

            // On sauvegarde la matrice courante
            glPushMatrix();

            // on choisit une couleur
            HSVtoRGB((duration*100 / 360.0 - floor(duration*100 / 360.0)) * 360.0, 100, 100, sauvColorR, sauvColorG, sauvColorB);
            std::cout << "duration : " << duration / 360.0 - floor(duration / 360.0) * 360 << std::endl;
            std::cout << "rouge : " << sauvColorR << " - vert : " << sauvColorG << " - bleu : " << sauvColorB << endl;

            if (m_Markers[m].id == 143) {
                glColor3f(sauvColorR, sauvColorG, sauvColorB);
                sauvX = m_Markers[m].getCenter().x;
                sauvY = m_Markers[m].getCenter().y;
                sauvR = m_Markers[m].getRadius();

                // On se deplace sur Z de la moitie du marqueur pour dessiner "sur" le plan du marqueur
                glTranslatef(0, 0, m_MarkerSize / 2.);
                drawChangingPlane(m_MarkerSize);
            }

            if (m_Markers[m].id == 186) {
                checkDistance(sauvX, sauvY, sauvR, m_Markers[m]);
                glColor3f(FixColorR, FixColorG, FixColorB);
                addForm(m_MarkerSize);
            }


            // On re=charge la matrice que l'on a sauvegarde
            glPopMatrix();
        }
    }

    // Desactivation du depth test
    glDisable(GL_DEPTH_TEST);
}



// Idle function
void ArUco::idle(Mat newImage) {
    // Getting new image
    m_InputImage = newImage.clone();

    // Undistort image based on distorsion parameters
    m_UndInputImage.create(m_InputImage.size(), CV_8UC3);

    //transform color that by default is BGR to RGB because windows systems do not allow reading BGR images with opengl properly
    cv::cvtColor(m_InputImage, m_InputImage, cv::COLOR_BGR2RGB);

    //remove distorion in image ==> does not work very well (the YML file is not that of my camera)
    //cv::undistort(m_InputImage,m_UndInputImage, m_CameraParams.CameraMatrix, m_CameraParams.Distorsion);
    m_UndInputImage = m_InputImage.clone();

    //resize the image to the size of the GL window
    cv::resize(m_UndInputImage, m_ResizedImage, m_GlWindowSize);

    //detect markers
    m_PPDetector.detect(m_ResizedImage, m_Markers, m_CameraParams, m_MarkerSize, false);

}

// Resize function
void ArUco::resize(GLsizei iWidth, GLsizei iHeight) {
    m_GlWindowSize = Size(iWidth, iHeight);

    //not all sizes are allowed. OpenCv images have padding at the end of each line in these that are not aligned to 4 bytes
    if (iWidth * 3 % 4 != 0) {
        iWidth += iWidth * 3 % 4;//resize to avoid padding
        resize(iWidth, m_GlWindowSize.height);
    }
    else {
        //resize the image to the size of the GL window
        if (m_UndInputImage.rows != 0)
            cv::resize(m_UndInputImage, m_ResizedImage, m_GlWindowSize);
    }

}

// Test using ArUco to display a 3D cube in OpenCV
void ArUco::draw3DCube(cv::Mat img, int markerInd) {
    if (m_Markers.size() > markerInd) {
        aruco::CvDrawingUtils::draw3dCube(img, m_Markers[markerInd], m_CameraParams);
    }
}

void ArUco::draw3DAxis(cv::Mat img, int markerInd) {
    if (m_Markers.size() > markerInd) {
        aruco::CvDrawingUtils::draw3dAxis(img, m_Markers[markerInd], m_CameraParams);
    }

}