#define _USE_MATH_DEFINES

#include <string>
#include <math.h>
#include <algorithm>
#include <map>
#include <list>
#include <GL/glew.h>

#include "GameTools.h"

using namespace std;
using namespace Gdiplus;
using namespace tools::image;
using namespace tools::audio;

namespace gametools {

    //////////////////////////////////////////////////////////////////////////////////////
    // RENDERERS

    namespace renderer {

        Renderer::Renderer() {
            igType = NULL_RENDERER;
        }

        unsigned int Renderer::getType() {
            return igType;
        }

        void Renderer::drawBorder(const SI3DPOINT& rpPosition, float fpAngle, const SF2DPOINT rpScale, const std::shared_ptr <CONST_BORDER_TIER> apBorder, unsigned char ipPointSize, RGBA rpColor) const {
            shared_ptr <gametools::container::ResourceManager> plManager = gametools::container::ResourceManager::getInstance();
            HDC hlScreen = plManager->getDeviceContext();
            if(hlScreen == NULL)
                throw string("AlphaRenderer::drawSprite: Screen Handle not initialized!");

            if(!apBorder)
                throw string("Renderer::drawBorder: Border Array cannot be null.");

            Graphics olEngine(hlScreen);
            for(size_t j = 0; j < apBorder->size(); j++)
                for(size_t i = 0; i < apBorder->at(j)->size(); i++) {
                    vector<Point> alPoints;
                    for(size_t p = 0; p < apBorder->at(j)->at(i)->size(); p++) {
                        SI3DPOINT rlPoint = utilities::getNewVector(apBorder->at(j)->at(i)->at(p).x, apBorder->at(j)->at(i)->at(p).y, 0, rpPosition.x, rpPosition.y, 0, fpAngle, rpScale.x, rpScale.y);
                        alPoints.push_back(Point(rlPoint.x, rlPoint.y));
                    }
                    if(alPoints.size() != 0)
                        alPoints.push_back(Point(alPoints.front()));
                    Pen olPen(Color(255, rpColor.r, rpColor.g, rpColor.b));
                    olEngine.DrawLines(&olPen, &alPoints.at(0), (int) alPoints.size());
                }
        }

        void ScreenRenderer::prepareScreen(unsigned int ipScreenWidth, unsigned int ipScreenHeight) {
            return;
        }

        void ScreenRenderer::drawScreen() const {
            return;
        }

        namespace gdi {

            GDIScreenRenderer::GDIScreenRenderer() {
                unsigned char ilMode = gametools::container::ResourceManager::getInstance()->getMode();
                if(ilMode != gametools::container::ResourceManager::GDI_ALPHA_MODE && ilMode != gametools::container::ResourceManager::GDI_MASK_MODE)
                    throw string("OGLScreenRenderer::OGLScreenRenderer: Not in GDI_ALPHA_MODE or GDI_MASK_MODE.");
            }

            void GDIScreenRenderer::prepareScreen(unsigned int ipScreenWidth, unsigned int ipScreenHeight) {
                return;
            }

            void GDIScreenRenderer::drawScreen() const {
                HWND hWnd = gametools::container::ResourceManager::getInstance()->getWindowHandle();
                InvalidateRect(hWnd, NULL, true);
                return;
            }

            AlphaRenderer::AlphaRenderer() {
                if(gametools::container::ResourceManager::getInstance()->getMode() != gametools::container::ResourceManager::GDI_ALPHA_MODE)
                    throw string("AlphaRenderer::AlphaRenderer: Only can create gametools::renderer::gdi::AlphaRenderer in GDI_ALPHA_MODE.");
                igType = GDI_ALPHA_RENDERER;
            }

            void AlphaRenderer::drawSprite(const SI3DPOINT& rpPosition, float fpAngle, const SF2DPOINT rpScale, size_t hpBitmap, unsigned long ipXDelta, unsigned long ipYDelta, unsigned int ipSpriteWidth, unsigned int ipSpriteHeight) const {
                shared_ptr <gametools::container::ResourceManager> plManager = gametools::container::ResourceManager::getInstance();
                HDC hlScreen = plManager->getDeviceContext();
                if(hlScreen == NULL)
                    throw string("AlphaRenderer::drawSprite: Screen Handle not initialized!");

                shared_ptr<RGBAImage> olBitmap = plManager->getBitmap(hpBitmap);

                Graphics olEngine(hlScreen);
                Status elStatus = olEngine.GetLastStatus();
                if(elStatus != S_OK)
                    throw string("AlphaRenderer::drawSprite: Initialization of GDI+ Engine failed.");
                Image* olImage = new Bitmap((int) olBitmap->getWidth(), (int) olBitmap->getHeight(), (int) olBitmap->getRawLineSize(), PixelFormat32bppARGB, (BYTE *) &olBitmap->getRawData()[0]);
                elStatus = olImage->GetLastStatus();
                if(elStatus != S_OK)
                    throw string("AlphaRenderer::drawSprite: GDI+ Image failed.");
                int ilHalfWidth = ipSpriteWidth / 2;
                int ilHalfHeight = ipSpriteHeight / 2;
                SI3DPOINT rlPoint1 = utilities::getNewVector(-ilHalfWidth, ilHalfHeight, 0, rpPosition.x, rpPosition.y, 0, fpAngle, rpScale.x, rpScale.y);
                SI3DPOINT rlPoint2 = utilities::getNewVector(ilHalfWidth, ilHalfHeight, 0, rpPosition.x, rpPosition.y, 0, fpAngle, rpScale.x, rpScale.y);
                SI3DPOINT rlPoint3 = utilities::getNewVector(-ilHalfWidth, -ilHalfHeight, 0, rpPosition.x, rpPosition.y, 0, fpAngle, rpScale.x, rpScale.y);
                Point  alDestPoints[3] = {
                    Point(rlPoint1.x, rlPoint1.y),
                    Point(rlPoint2.x, rlPoint2.y),
                    Point(rlPoint3.x, rlPoint3.y)
                };

                elStatus = olEngine.DrawImage(
                    olImage,
                    alDestPoints,
                    3,
                    ipXDelta, ipYDelta,
                    ipSpriteWidth, ipSpriteHeight,
                    UnitPixel,
                    NULL,
                    NULL,
                    NULL
                );

                if(elStatus != S_OK)
                    throw string("AlphaRenderer::drawSprite: GDI+ Engine failed.");
            }

            MaskRenderer::MaskRenderer() {
                if(gametools::container::ResourceManager::getInstance()->getMode() != gametools::container::ResourceManager::GDI_MASK_MODE)
                    throw string("MaskRenderer::MaskRenderer: Only can create gametools::renderer::gdi::MaskRenderer in GDI_MASK_MODE.");
                igType = GDI_MASK_RENDERER;
            }

            void MaskRenderer::drawSprite(const SI3DPOINT& rpPosition, float fpAngle, const SF2DPOINT rpScale, size_t hpBitmap, unsigned long ipXDelta, unsigned long ipYDelta, unsigned int ipSpriteWidth, unsigned int ipSpriteHeight) const {
                shared_ptr <gametools::container::ResourceManager> plManager = gametools::container::ResourceManager::getInstance();
                HDC hlScreen = plManager->getDeviceContext();
                if(hlScreen == NULL)
                    throw string("MaskRenderer::drawSprite: Screen Handle not initialized!");

                AlphaRenderer olRenderer;

                olRenderer.drawSprite(rpPosition, fpAngle, rpScale, plManager->getMaskIndex(hpBitmap), ipXDelta, ipYDelta, ipSpriteWidth, ipSpriteHeight);
                olRenderer.drawSprite(rpPosition, fpAngle, rpScale, hpBitmap, ipXDelta, ipYDelta, ipSpriteWidth, ipSpriteHeight);
            }

        } // END RENDERER::GDI

        namespace ogl {
            shared_ptr <vector <float>> OGLRenderer::agPerspectiveMatrix = make_shared <vector <float>>(16, 0.0f);
            float OGLRenderer::fgPixelSize = 0.0f;
            float OGLRenderer::fgAspect = 0.0f;

            OGLRenderer::OGLRenderer() {
                igType = NULL_RENDERER;
                hgProgram = -1;
                sgVertexShader = "";
                sgGeometryShader = "";
                sgFragmentShader = "";
            }

            const size_t OGLRenderer::DEFAULT_PROGRAM1 = 0;
            const size_t OGLRenderer::DEFAULT_PROGRAM2 = 1;
            const float OGLRenderer::NORMALIZED_COLOR_UNIT = 1.0f / 255.0f;

            OGLRenderer::~OGLRenderer() {
                if(hgProgram != -1)
                    glDeleteProgram(hgProgram);
            }

            void OGLRenderer::drawBorder(const SI3DPOINT& rpPosition, float fpAngle, const SF2DPOINT rpScale, const shared_ptr <CONST_BORDER_TIER> apBorder, unsigned char ipPointSize, RGBA rpColor) const {
                if(!apBorder)
                    throw string("OGLRenderer::drawBorder: Border Array cannot be null.");

                shared_ptr <vector <float>> alTransformation = utilities::getIdentityMatrix();
                std::vector <float> alBuffer = vector <float>();
                for(size_t i = 0; i < apBorder->size(); i++)
                    for(size_t j = 0; j < apBorder->at(i)->size(); j++)
                        for(size_t k = 0; k < apBorder->at(i)->at(j)->size(); k++) {
                            SI3DPOINT rlTmpPoint = utilities::getNewVector(
                                apBorder->at(i)->at(j)->at(k).x,
                                apBorder->at(i)->at(j)->at(k).y,
                                apBorder->at(i)->at(j)->at(k).z,
                                rpPosition.x,
                                rpPosition.y,
                                rpPosition.z,
                                fpAngle,
                                rpScale.x,
                                rpScale.y
                            );
                            alBuffer.push_back(((float) rlTmpPoint.x * fgPixelSize) - fgAspect);
                            alBuffer.push_back(((float) rlTmpPoint.y * fgPixelSize * -1.0f) + 1.0f);
                            alBuffer.push_back(0.0f);
                            alBuffer.push_back(0.0f);
                            alBuffer.push_back(0.0f);
                        }

                unsigned int hlPointVertexArray;
                unsigned int hlPointElements;
                unsigned int hlPoints;
                glGenVertexArrays(1, &hlPointVertexArray);
                glGenBuffers(1, &hlPointElements);
                glGenBuffers(1, &hlPoints);
                glBindVertexArray(hlPointVertexArray);

                vector <unsigned int> alElementBuffer;
                for(size_t i = 0; i < alBuffer.size() / 5; i++)
                    alElementBuffer.push_back((unsigned int) i);

                glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, hlPointElements);
                glBufferData(GL_ELEMENT_ARRAY_BUFFER, alElementBuffer.size() * sizeof(unsigned int), &alElementBuffer[0], GL_STATIC_DRAW);


                // Allocate space and upload the data from CPU to GPU
                glBindBuffer(GL_ARRAY_BUFFER, hlPoints);
                glBufferData(GL_ARRAY_BUFFER, sizeof(float) * alBuffer.size(), &alBuffer[0], GL_STATIC_DRAW);

                // position attribute
                glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*) 0);
                glEnableVertexAttribArray(0);
                // texture coord attribute
                glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*) (3 * sizeof(float)));
                glEnableVertexAttribArray(1);

                glEnable(GL_VERTEX_PROGRAM_POINT_SIZE);
                glUseProgram(hgProgram);
                glUniformMatrix4fv(glGetUniformLocation(hgProgram, "aPerspective"), 1, false, &OGLRenderer::getPerspetiveMatrix()->at(0));
                glUniformMatrix4fv(glGetUniformLocation(hgProgram, "aTransformation"), 1, false, &alTransformation->at(0));
                glUniform1f(glGetUniformLocation(hgProgram, "aPointSize"), (float) ipPointSize);
                glUniform3f(glGetUniformLocation(hgProgram, "aPointColor"), (float) rpColor.r * NORMALIZED_COLOR_UNIT, (float) rpColor.g * NORMALIZED_COLOR_UNIT, (float) rpColor.b * NORMALIZED_COLOR_UNIT);
                glBindVertexArray(hlPointVertexArray);
                glDrawElements(GL_LINE_LOOP, (unsigned int) alElementBuffer.size(), GL_UNSIGNED_INT, 0);
                glDeleteBuffers(1, &hlPoints);
                glDeleteBuffers(1, &hlPointElements);
                glDeleteVertexArrays(1, &hlPointVertexArray);
            }

            std::string OGLRenderer::getVertexShaderSourceCode() const {
                return sgVertexShader;
            }

            std::string OGLRenderer::getGeometryShaderSourceCode() const {
                return sgGeometryShader;
            }

            std::string OGLRenderer::getFragmentShaderSourceCode() const {
                return sgFragmentShader;
            }

            void OGLRenderer::setPerspectiveMatrix(float fpFieldOfView, unsigned int ipWidth, unsigned int ipHeight, float fpZNear, float fpZFar) {
                fgAspect = (float) ipWidth / (float) ipHeight;
                fgPixelSize = 2.0f / (float) ipHeight;
                agPerspectiveMatrix = utilities::getPerspectiveMatrix(fpFieldOfView, fgAspect, fpZNear, fpZFar);
            }

            void OGLRenderer::setPerspectiveMatrixFromWindow(HWND hpWindow) {
                if(hpWindow == NULL)
                    throw string("OGLRenderer::setPerspectiveMatrixFromWindow: Window Handle cannot be null.");

                RECT rlWindowDescriptor;
                GetClientRect(hpWindow, &rlWindowDescriptor);
                glViewport(0, 0, rlWindowDescriptor.right, rlWindowDescriptor.bottom);
                OGLRenderer::setPerspectiveMatrix(90.0f, rlWindowDescriptor.right, rlWindowDescriptor.bottom, 0.00001f, 1.0f);
            }

            shared_ptr <vector <float>> OGLRenderer::getPerspetiveMatrix() {
                return agPerspectiveMatrix;
            }

            float OGLRenderer::getPixelSize() {
                return fgPixelSize;
            }

            OGLScreenRenderer::OGLScreenRenderer() {
                unsigned char ilMode = gametools::container::ResourceManager::getInstance()->getMode();
                if(ilMode != gametools::container::ResourceManager::OGL_ALPHA_MODE && ilMode != gametools::container::ResourceManager::OGL_MASK_MODE)
                    throw string("OGLScreenRenderer::OGLScreenRenderer: Not in OGL_ALPHA_MODE or OGL_MASK_MODE.");
            }

            void OGLScreenRenderer::prepareScreen(unsigned int ipScreenWidth, unsigned int ipScreenHeight) {
                glClear(GL_COLOR_BUFFER_BIT);   // Clear the color buffer with current clearing color
                return;
            }

            void OGLScreenRenderer::drawScreen() const {
                SwapBuffers(gametools::container::ResourceManager::getInstance()->getDeviceContext());
                glFinish();  // Esto debe ir después de SWAPBUFFERS para que este Últimmo se sincronice con VSYNC.
                return;
            }

            AlphaRenderer::AlphaRenderer() {
                if(gametools::container::ResourceManager::getInstance()->getMode() != gametools::container::ResourceManager::OGL_ALPHA_MODE)
                    throw string("AlphaRenderer::AlphaRenderer: Only can create gametools::renderer::ogl::AlphaSprite in OGL_ALPHA_MODE.");

                hgProgram = gametools::container::ResourceManager::getInstance()->getShaderProgramHandle(DEFAULT_PROGRAM1);

                agElementIndexes = {
                            0, 1, 3,
                            1, 2, 3
                };

                glGenVertexArrays(1, &hgVertexArray);
                glGenBuffers(1, &hgVertexBuffer);
                glGenBuffers(1, &hgElementBuffer);

                glBindVertexArray(hgVertexArray);

                glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, hgElementBuffer);
                glBufferData(GL_ELEMENT_ARRAY_BUFFER, agElementIndexes.size() * sizeof(unsigned int), &agElementIndexes[0], GL_STATIC_DRAW);

                igType = OGL_ALPHA_RENDERER;
            }

            AlphaRenderer::~AlphaRenderer() {
                glDeleteBuffers(1, &hgElementBuffer);
                glDeleteBuffers(1, &hgVertexBuffer);
                glDeleteVertexArrays(1, &hgVertexArray);
            }

            void AlphaRenderer::drawSprite(const SI3DPOINT& rpPosition, float fpAngle, const SF2DPOINT rpScale, size_t hpBitmap, unsigned long ipXDelta, unsigned long ipYDelta, unsigned int ipSpriteWidth, unsigned int ipSpriteHeight) const {
                shared_ptr <gametools::container::ResourceManager> olManager = gametools::container::ResourceManager::getInstance();
                shared_ptr <RGBAImage> olBitmap = olManager->getBitmap(hpBitmap);
                shared_ptr <vector <float>> alTransformation = utilities::getTransformationMatrix(((float) rpPosition.x * fgPixelSize) - fgAspect, ((float) rpPosition.y * fgPixelSize * -1.0f) + 1.0f, (float) rpPosition.z * fgPixelSize, fpAngle, rpScale.x, rpScale.y);

                std::vector <float> alBuffer = vector <float>(20, 0.0f);

                float flWWidth = ((float) ipSpriteWidth * fgPixelSize) / 2;
                float flWHeight = ((float) ipSpriteHeight * fgPixelSize) / 2;
                alBuffer[0] = flWWidth;
                alBuffer[1] = flWHeight;
                alBuffer[2] = 0;
                alBuffer[5] = flWWidth;
                alBuffer[6] = -flWHeight;
                alBuffer[7] = 0;
                alBuffer[10] = -flWWidth;
                alBuffer[11] = -flWHeight;
                alBuffer[12] = 0;
                alBuffer[15] = -flWWidth;
                alBuffer[16] = flWHeight;
                alBuffer[17] = 0;

                float flTXPixelSize = 1.0f / (float) olBitmap->getWidth();
                float flTYPixelSize = 1.0f / (float) olBitmap->getHeight();
                float flTX = (float) ipXDelta * flTXPixelSize;
                float flTY = (float) ipYDelta * flTYPixelSize;
                float flTWidth = (float) ipSpriteWidth * flTXPixelSize;
                float flTHeight = (float) ipSpriteHeight * flTYPixelSize;
                alBuffer[3] = flTX + flTWidth;
                alBuffer[4] = flTY;
                alBuffer[8] = flTX + flTWidth;
                alBuffer[9] = flTY - flTHeight;
                alBuffer[13] = flTX;
                alBuffer[14] = flTY - flTHeight;
                alBuffer[18] = flTX;
                alBuffer[19] = flTY;

                glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
                glActiveTexture(GL_TEXTURE0);
                glBindTexture(GL_TEXTURE_2D, (unsigned int) olManager->getBitmapHandle(hpBitmap));

                glBindVertexArray(hgVertexArray);
                glBindBuffer(GL_ARRAY_BUFFER, hgVertexBuffer);
                glBufferData(GL_ARRAY_BUFFER, alBuffer.size() * sizeof(float), &alBuffer[0], GL_DYNAMIC_DRAW);

                // position attribute
                glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*) 0);
                glEnableVertexAttribArray(0);
                // texture coord attribute
                glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*) (3 * sizeof(float)));
                glEnableVertexAttribArray(1);

                glUseProgram(hgProgram);
                glUniformMatrix4fv(glGetUniformLocation(hgProgram, "aPerspective"), 1, false, &OGLRenderer::getPerspetiveMatrix()->at(0));
                glUniformMatrix4fv(glGetUniformLocation(hgProgram, "aTransformation"), 1, false, &alTransformation->at(0));
                glUniform1f(glGetUniformLocation(hgProgram, "aPointSize"), 0.0f);
                glBindVertexArray(hgVertexArray);
                glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
            }

            MaskByColorRenderer::MaskByColorRenderer(const tools::image::RGBA& rpColor) {
                if(gametools::container::ResourceManager::getInstance()->getMode() != gametools::container::ResourceManager::OGL_MASK_MODE)
                    throw string("MaskRendererByColor::MaskRendererByColor: Only can create gametools::renderer::ogl::MaskRendererByColor in OGL_MASK_MODE.");

                rgMaskColor = rpColor;
                hgProgram = gametools::container::ResourceManager::getInstance()->getShaderProgramHandle(DEFAULT_PROGRAM2);

                agElementIndexes = {
                            0, 1, 3,
                            1, 2, 3
                };

                glGenVertexArrays(1, &hgVertexArray);
                glGenBuffers(1, &hgVertexBuffer);
                glGenBuffers(1, &hgElementBuffer);

                glBindVertexArray(hgVertexArray);

                glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, hgElementBuffer);
                glBufferData(GL_ELEMENT_ARRAY_BUFFER, agElementIndexes.size() * sizeof(float), &agElementIndexes[0], GL_STATIC_DRAW);

                rgMaskColor = {255, 255, 255, 255};

                igType = OGL_MASK_RENDERER;
            }

            MaskByColorRenderer::~MaskByColorRenderer() {
                glDeleteBuffers(1, &hgElementBuffer);
                glDeleteBuffers(1, &hgVertexBuffer);
                glDeleteVertexArrays(1, &hgVertexArray);
            }

            void MaskByColorRenderer::drawSprite(const SI3DPOINT& rpPosition, float fpAngle, const SF2DPOINT rpScale, size_t hpBitmap, unsigned long ipXDelta, unsigned long ipYDelta, unsigned int ipSpriteWidth, unsigned int ipSpriteHeight) const {
                shared_ptr <gametools::container::ResourceManager> olManager = gametools::container::ResourceManager::getInstance();
                shared_ptr <RGBAImage> olBitmap = olManager->getBitmap(hpBitmap);
                shared_ptr <vector <float>> alTransformation = utilities::getTransformationMatrix(((float) rpPosition.x * fgPixelSize) - fgAspect, ((float) rpPosition.y * fgPixelSize * -1.0f) + 1.0f, (float) rpPosition.z * fgPixelSize, fpAngle, rpScale.x, rpScale.y);

                std::vector <float> alBuffer = vector <float>(20, 0.0f);

                float flWWidth = ((float) ipSpriteWidth * fgPixelSize) / 2;
                float flWHeight = ((float) ipSpriteHeight * fgPixelSize) / 2;
                alBuffer[0] = flWWidth;
                alBuffer[1] = flWHeight;
                alBuffer[2] = 0;
                alBuffer[5] = flWWidth;
                alBuffer[6] = -flWHeight;
                alBuffer[7] = 0;
                alBuffer[10] = -flWWidth;
                alBuffer[11] = -flWHeight;
                alBuffer[12] = 0;
                alBuffer[15] = -flWWidth;
                alBuffer[16] = flWHeight;
                alBuffer[17] = 0;

                float flTXPixelSize = 1.0f / (float) olBitmap->getWidth();
                float flTYPixelSize = 1.0f / (float) olBitmap->getHeight();
                float flTX = (float) ipXDelta * flTXPixelSize;
                float flTY = (float) ipYDelta * flTYPixelSize;
                float flTWidth = (float) ipSpriteWidth * flTXPixelSize;
                float flTHeight = (float) ipSpriteHeight * flTYPixelSize;
                alBuffer[3] = flTX + flTWidth;
                alBuffer[4] = flTY;
                alBuffer[8] = flTX + flTWidth;
                alBuffer[9] = flTY - flTHeight;
                alBuffer[13] = flTX;
                alBuffer[14] = flTY - flTHeight;
                alBuffer[18] = flTX;
                alBuffer[19] = flTY;

                glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
                glActiveTexture(GL_TEXTURE0);
                glBindTexture(GL_TEXTURE_2D, (unsigned int) olManager->getBitmapHandle(hpBitmap));

                glBindVertexArray(hgVertexArray);
                glBindBuffer(GL_ARRAY_BUFFER, hgVertexBuffer);
                glBufferData(GL_ARRAY_BUFFER, alBuffer.size() * sizeof(float), &alBuffer[0], GL_DYNAMIC_DRAW);

                // position attribute
                glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*) 0);
                glEnableVertexAttribArray(0);
                // texture coord attribute
                glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*) (3 * sizeof(float)));
                glEnableVertexAttribArray(1);

                glUseProgram(hgProgram);
                glUniformMatrix4fv(glGetUniformLocation(hgProgram, "aPerspective"), 1, false, &OGLRenderer::getPerspetiveMatrix()->at(0));
                glUniformMatrix4fv(glGetUniformLocation(hgProgram, "aTransformation"), 1, false, &alTransformation->at(0));
                glUniform1f(glGetUniformLocation(hgProgram, "aPointSize"), 0.0f);
                glUniform3f(glGetUniformLocation(hgProgram, "aMaskColor"), (float) rgMaskColor.r * NORMALIZED_COLOR_UNIT, (float) rgMaskColor.g * NORMALIZED_COLOR_UNIT, (float) rgMaskColor.b * NORMALIZED_COLOR_UNIT);
                glBindVertexArray(hgVertexArray);
                glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
            }

            tools::image::RGBA MaskByColorRenderer::getMaskColor() const {
                return rgMaskColor;
            }

            MaskRenderer::MaskRenderer() {
                if(gametools::container::ResourceManager::getInstance()->getMode() != gametools::container::ResourceManager::OGL_MASK_MODE)
                    throw string("MaskRenderer::MaskRenderer: Only can create gametools::renderer::ogl::MaskRenderer in OGL_MASK_MODE.");

                hgProgram = gametools::container::ResourceManager::getInstance()->getShaderProgramHandle(DEFAULT_PROGRAM1);

                agElementIndexes = {
                            0, 1, 3,
                            1, 2, 3
                };

                glGenVertexArrays(1, &hgVertexArray);
                glGenBuffers(1, &hgVertexBuffer);
                glGenBuffers(1, &hgElementBuffer);

                glBindVertexArray(hgVertexArray);

                glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, hgElementBuffer);
                glBufferData(GL_ELEMENT_ARRAY_BUFFER, agElementIndexes.size() * sizeof(float), &agElementIndexes[0], GL_STATIC_DRAW);

                igType = OGL_MASK_RENDERER;
            }

            MaskRenderer::~MaskRenderer() {
                glDeleteBuffers(1, &hgElementBuffer);
                glDeleteBuffers(1, &hgVertexBuffer);
                glDeleteVertexArrays(1, &hgVertexArray);
            }

            void MaskRenderer::drawSprite(const SI3DPOINT& rpPosition, float fpAngle, const SF2DPOINT rpScale, size_t hpBitmap, unsigned long ipXDelta, unsigned long ipYDelta, unsigned int ipSpriteWidth, unsigned int ipSpriteHeight) const {
                shared_ptr <gametools::container::ResourceManager> olManager = gametools::container::ResourceManager::getInstance();
                shared_ptr <RGBAImage> olBitmap = olManager->getBitmap(hpBitmap);
                shared_ptr <vector <float>> alTransformation = utilities::getTransformationMatrix(((float) rpPosition.x * fgPixelSize) - fgAspect, ((float) rpPosition.y * fgPixelSize * -1.0f) + 1.0f, (float) rpPosition.z * fgPixelSize, fpAngle, rpScale.x, rpScale.y);

                std::vector <float> alBuffer = vector <float> (20, 0.0f);

                float flWWidth = ((float) ipSpriteWidth * fgPixelSize) / 2;
                float flWHeight = ((float) ipSpriteHeight * fgPixelSize) / 2;
                alBuffer[0] = flWWidth;
                alBuffer[1] = flWHeight;
                alBuffer[2] = 0;
                alBuffer[5] = flWWidth;
                alBuffer[6] = -flWHeight;
                alBuffer[7] = 0;
                alBuffer[10] = -flWWidth;
                alBuffer[11] = -flWHeight;
                alBuffer[12] = 0;
                alBuffer[15] = -flWWidth;
                alBuffer[16] = flWHeight;
                alBuffer[17] = 0;

                float flTXPixelSize = 1.0f / (float) olBitmap->getWidth();
                float flTYPixelSize = 1.0f / (float) olBitmap->getHeight();
                float flTX = (float) ipXDelta * flTXPixelSize;
                float flTY = (float) ipYDelta * flTYPixelSize;
                float flTWidth = (float) ipSpriteWidth * flTXPixelSize;
                float flTHeight = (float) ipSpriteHeight * flTYPixelSize;
                alBuffer[3] = flTX + flTWidth;
                alBuffer[4] = flTY;
                alBuffer[8] = flTX + flTWidth;
                alBuffer[9] = flTY - flTHeight;
                alBuffer[13] = flTX;
                alBuffer[14] = flTY - flTHeight;
                alBuffer[18] = flTX;
                alBuffer[19] = flTY;

                unsigned int hlMask = (unsigned int) olManager->getBitmapHandle(olManager->getMaskIndex(hpBitmap));

                glBlendFunc(GL_ONE, GL_ONE);
                glActiveTexture(GL_TEXTURE0);
                glBindTexture(GL_TEXTURE_2D, hlMask);

                glBindVertexArray(hgVertexArray);
                glBindBuffer(GL_ARRAY_BUFFER, hgVertexBuffer);
                glBufferData(GL_ARRAY_BUFFER, alBuffer.size() * sizeof(float), &alBuffer[0], GL_DYNAMIC_DRAW);

                // position attribute
                glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*) 0);
                glEnableVertexAttribArray(0);
                // texture coord attribute
                glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*) (3 * sizeof(float)));
                glEnableVertexAttribArray(1);

                glUseProgram(hgProgram);
                glUniformMatrix4fv(glGetUniformLocation(hgProgram, "aPerspective"), 1, false, &OGLRenderer::getPerspetiveMatrix()->at(0));
                glUniformMatrix4fv(glGetUniformLocation(hgProgram, "aTransformation"), 1, false, &alTransformation->at(0));
                glUniform1f(glGetUniformLocation(hgProgram, "aPointSize"), 0.0f);
                glBindVertexArray(hgVertexArray);
                glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
                
                ////////////////////////////////////////////////////////////////////////////////////////
                glBlendColor(0.0f, 0.0f, 0.0f, 1.0f);
                glBlendFunc(GL_CONSTANT_COLOR, GL_ONE_MINUS_SRC_COLOR);
                glActiveTexture(GL_TEXTURE0);
                glBindTexture(GL_TEXTURE_2D, hlMask);

                glBindVertexArray(hgVertexArray);
                glBindBuffer(GL_ARRAY_BUFFER, hgVertexBuffer);
                glBufferData(GL_ARRAY_BUFFER, alBuffer.size() * sizeof(float), &alBuffer[0], GL_DYNAMIC_DRAW);

                // position attribute
                glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*) 0);
                glEnableVertexAttribArray(0);
                // texture coord attribute
                glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*) (3 * sizeof(float)));
                glEnableVertexAttribArray(1);

                glUseProgram(hgProgram);
                glUniformMatrix4fv(glGetUniformLocation(hgProgram, "aPerspective"), 1, false, &OGLRenderer::getPerspetiveMatrix()->at(0));
                glUniform1f(glGetUniformLocation(hgProgram, "aPointSize"), 0.0f);
                glBindVertexArray(hgVertexArray);
                glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

                ////////////////////////////////////////////////////////////////////////////////////////
                glBlendFunc(GL_ONE, GL_ONE);
                glActiveTexture(GL_TEXTURE0);
                glBindTexture(GL_TEXTURE_2D, hlMask);

                glBindVertexArray(hgVertexArray);
                glBindBuffer(GL_ARRAY_BUFFER, hgVertexBuffer);
                glBufferData(GL_ARRAY_BUFFER, alBuffer.size() * sizeof(float), &alBuffer[0], GL_DYNAMIC_DRAW);

                // position attribute
                glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*) 0);
                glEnableVertexAttribArray(0);
                // texture coord attribute
                glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*) (3 * sizeof(float)));
                glEnableVertexAttribArray(1);

                glUseProgram(hgProgram);
                glUniformMatrix4fv(glGetUniformLocation(hgProgram, "aPerspective"), 1, false, &OGLRenderer::getPerspetiveMatrix()->at(0));
                glUniform1f(glGetUniformLocation(hgProgram, "aPointSize"), 0.0f);
                glBindVertexArray(hgVertexArray);
                glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
                
                ////////////////////////////////////////////////////////////////////////////////////////
                glBlendColor(0.0f, 0.0f, 0.0f, 1.0f);
                glBlendFunc(GL_CONSTANT_COLOR, GL_ONE_MINUS_SRC_COLOR);
                glActiveTexture(GL_TEXTURE0);
                glBindTexture(GL_TEXTURE_2D, hlMask);

                glBindVertexArray(hgVertexArray);
                glBindBuffer(GL_ARRAY_BUFFER, hgVertexBuffer);
                glBufferData(GL_ARRAY_BUFFER, alBuffer.size() * sizeof(float), &alBuffer[0], GL_DYNAMIC_DRAW);

                // position attribute
                glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*) 0);
                glEnableVertexAttribArray(0);
                // texture coord attribute
                glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*) (3 * sizeof(float)));
                glEnableVertexAttribArray(1);

                glUseProgram(hgProgram);
                glUniformMatrix4fv(glGetUniformLocation(hgProgram, "aPerspective"), 1, false, &OGLRenderer::getPerspetiveMatrix()->at(0));
                glUniform1f(glGetUniformLocation(hgProgram, "aPointSize"), 0.0f);
                glBindVertexArray(hgVertexArray);
                glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
                
                ////////////////////////////////////////////////////////////////////////////////////////
                glBlendFunc(GL_ONE, GL_ONE);
                glActiveTexture(GL_TEXTURE0);
                glBindTexture(GL_TEXTURE_2D, (unsigned int) olManager->getBitmapHandle(hpBitmap));

                glBindVertexArray(hgVertexArray);
                glBindBuffer(GL_ARRAY_BUFFER, hgVertexBuffer);
                glBufferData(GL_ARRAY_BUFFER, alBuffer.size() * sizeof(float), &alBuffer[0], GL_DYNAMIC_DRAW);

                // position attribute
                glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*) 0);
                glEnableVertexAttribArray(0);
                // texture coord attribute
                glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*) (3 * sizeof(float)));
                glEnableVertexAttribArray(1);

                glUseProgram(hgProgram);
                glUniformMatrix4fv(glGetUniformLocation(hgProgram, "aPerspective"), 1, false, &OGLRenderer::getPerspetiveMatrix()->at(0));
                glUniform1f(glGetUniformLocation(hgProgram, "aPointSize"), 0.0f);
                glBindVertexArray(hgVertexArray);
                glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
            }

        } // END RENDERER::OGL

    } // END RENDERER

    //////////////////////////////////////////////////////////////////////////////////////
    // SPRITES

    namespace sprite {

        shared_ptr <vector <Sprite::FrameDescriptor>> Sprite::EMPTY_FRAMES = make_shared <vector <Sprite::FrameDescriptor>> ();

        Sprite::Sprite(shared_ptr <vector <FrameDescriptor>> apFrames) {
            igDivisionCount = 1;
            igDivisionWidth = 1;
            igDivisionHeight = 1;
            rgDivisionOrigin = {0, 0};
            igDivisionDirection = RIGHTWARD;
            rgBackgroundColor = BLACK;

            agFrames = make_shared <vector <FrameDescriptor>> ();
            for(size_t i = 0; i < apFrames->size(); i++) {
                if(!apFrames->at(i).active_borders)
                    throw string("Sprite::Sprite: Active Borders Vector is null in Frame.");
                if(!apFrames->at(i).passive_borders)
                    throw string("Sprite::Sprite: Passive Borders Vector is null in Frame.");

                FrameDescriptor rlTmp;
                rlTmp.division_inx = 0;
                rlTmp.delta_pos = {0, 0};
                rlTmp.tick_count = 0;
                rlTmp.tick_length = chrono::milliseconds(0);
                rlTmp.division_pos = {0, 0};
                rlTmp.active_borders = make_shared <BORDER_TIER> (*apFrames->at(i).active_borders);
                rlTmp.passive_borders = make_shared <BORDER_TIER> (*apFrames->at(i).passive_borders);

                agFrames->push_back(rlTmp);
            }
        }

        Sprite::Sprite(
            size_t hpImage,
            UI3DPOINT& rpDivisionOrigin,
            unsigned long ipDivisionWidth,
            unsigned long ipDivisionHeight,
            unsigned char ipDivisionDirection,
            unsigned long ipDivisionCount,
            shared_ptr <vector <FrameDescriptor>> apFrames,
            const RGBA& ipBackgroundColor
        ) {
            if(!apFrames)
                throw string("Sprite::Sprite: Frame Data is null.");

            if(apFrames->size() == 0)
                throw string("Sprite::Sprite: Frame Data Vector must have at least one Record.");

            if(ipDivisionDirection == REVERSE)
                throw string("Sprite::Sprite: Cannot reverse.");

            if(ipDivisionCount == 0)
                throw string("Sprite::Sprite: Sprite must have at least 1 Frame.");

            shared_ptr <RGBAImage> olTmpImage = gametools::container::ResourceManager::getInstance()->getBitmap(hpImage);
            if(rpDivisionOrigin.x >= olTmpImage->getWidth())
                throw string("Sprite::Sprite: X Coordinate cannot be major than the Width of the Sprite.");

            if((ipDivisionDirection == RIGHTWARD || ipDivisionDirection == LEFTWARD) && rpDivisionOrigin.x + (ipDivisionWidth * ipDivisionCount) - 1 >= olTmpImage->getWidth())
                throw string("Sprite::Sprite: Frames do not fit into the Space determined by the Frame Width and the Frame Count from the X Source.");

            if(rpDivisionOrigin.y >= olTmpImage->getHeight())
                throw string("Sprite::Sprite: Y Coordinate cannot be major than the Height of the Sprite.");

            if((ipDivisionDirection == UPWARD || ipDivisionDirection == DOWNWARD) && rpDivisionOrigin.y + (ipDivisionHeight * ipDivisionCount) - 1 >= olTmpImage->getHeight())
                throw string("Sprite::Sprite: Frames do not fit into the Space determined by the Frame Height and the Frame Count from the Y Source.");

            hgBitmap = hpImage;
            igDivisionCount = ipDivisionCount;
            igDivisionWidth = ipDivisionWidth;
            igDivisionHeight = ipDivisionHeight;
            igDivisionDirection = ipDivisionDirection;
            rgDivisionOrigin = rpDivisionOrigin;
            rgBackgroundColor = ipBackgroundColor;

            agFrames = make_shared <vector <FrameDescriptor>> ();
            for(size_t i = 0; i < apFrames->size(); i++) {
                if(apFrames->at(i).division_inx >= igDivisionCount)
                    throw string("Sprite::Sprite: Division Index out of Bounds in Frame.");
                if(!apFrames->at(i).active_borders)
                    throw string("Sprite::Sprite: Active Borders Vector is null in Frame.");
                if(!apFrames->at(i).passive_borders)
                    throw string("Sprite::Sprite: Passive Borders Vector is null in Frame.");
                if((apFrames->at(i).marker & Sprite::MARK_FOR) && (apFrames->at(i).marker & Sprite::MARK_JUMP))
                    throw string("Sprite::Sprite: MARK_FOR and MARK_JUMP cannot be used in the Same Frame.");
                if((apFrames->at(i).marker & Sprite::MARK_FOR) && (apFrames->at(i).param == 0))
                    throw string("Sprite::Sprite: The Value of param cannot be 0 for MARK_FOR.");
                if((apFrames->at(i).marker & Sprite::MARK_JUMP) && (apFrames->at(i).param >= apFrames->size()))
                    throw string("Sprite::Sprite: The Value of param cannot be major than or equal to the Number of Frames.");

                FrameDescriptor rlTmp;
                rlTmp.division_inx = apFrames->at(i).division_inx;
                rlTmp.division_pos = apFrames->at(i).division_pos;
                rlTmp.tick_count = apFrames->at(i).tick_count;
                rlTmp.tick_length = apFrames->at(i).tick_length;
                rlTmp.delta_pos = apFrames->at(i).delta_pos;
                rlTmp.delta_scale = apFrames->at(i).delta_scale;
                rlTmp.delta_angle = apFrames->at(i).delta_angle;
                rlTmp.marker = apFrames->at(i).marker;
                rlTmp.param = apFrames->at(i).param;
                rlTmp.active_borders = make_shared <BORDER_TIER> (*apFrames->at(i).active_borders);
                rlTmp.passive_borders = make_shared <BORDER_TIER> (*apFrames->at(i).passive_borders);

                unsigned long ilDivisionX;
                unsigned long ilDivisionY;
                if(igDivisionDirection == RIGHTWARD) {
                    ilDivisionX = rpDivisionOrigin.x + (rlTmp.division_inx * igDivisionWidth);
                    ilDivisionY = rpDivisionOrigin.y;
                } else if(igDivisionDirection == LEFTWARD) {
                    ilDivisionX = rpDivisionOrigin.x + ((igDivisionCount - rlTmp.division_inx - 1) * igDivisionWidth);
                    ilDivisionY = rpDivisionOrigin.y;
                } else if(igDivisionDirection == DOWNWARD) {
                    ilDivisionX = rpDivisionOrigin.x;
                    ilDivisionY = rpDivisionOrigin.y + ((igDivisionCount - rlTmp.division_inx - 1) * igDivisionHeight);
                } else if(igDivisionDirection == UPWARD) {
                    ilDivisionX = rpDivisionOrigin.x;
                    ilDivisionY = rpDivisionOrigin.y + (rlTmp.division_inx * igDivisionHeight);
                }
                rlTmp.division_pos.x = ilDivisionX;
                rlTmp.division_pos.y = ilDivisionY;

                agFrames->push_back(rlTmp);
            }
        }

        std::shared_ptr <const std::vector <Sprite::FrameDescriptor>>  Sprite::getFrames() const {
            return agFrames;
        }

        size_t Sprite::getBitmapHandle() const {
            return hgBitmap;
        }

        unsigned long Sprite::getDivisionCount() const {
            return igDivisionCount;
        }

        size_t Sprite::getFrameCount() const {
            return agFrames->size();
        }

        unsigned int Sprite::getDivisionHeight() const {
            return igDivisionHeight;
        }

        unsigned int Sprite::getDivisionWidth() const {
            return igDivisionWidth;
        }

        const UI3DPOINT& Sprite::getDivisionOrigin() const {
            return rgDivisionOrigin;
        }

        unsigned char Sprite::getDivisionDirection() const {
            return igDivisionDirection;
        }

        const RGBA& Sprite::getBackgroundColor() const {
            return rgBackgroundColor;
        }

        shared_ptr <RGBAImage> SpriteCreator::getFrameBitmap(shared_ptr <Sprite> opSprite, unsigned long ipFrameIndex, bool blMask) {
            if(!opSprite)
                throw string("SpriteCreator::getFrameBitmap: Sprite cannot be null.");
            if(ipFrameIndex >= opSprite->getFrameCount())
                throw string("SpriteCreator::getFrameBitmap: Invalid Frame Index.");
            unsigned long ilDivisionInx = opSprite->getFrames()->at(ipFrameIndex).division_inx;

            shared_ptr <gametools::container::ResourceManager> olManager = gametools::container::ResourceManager::getInstance();
            shared_ptr <RGBAImage> olBitmap = blMask ? olManager->getBitmap(olManager->getMaskIndex(opSprite->getBitmapHandle())): olManager->getBitmap(opSprite->getBitmapHandle());
            if(olBitmap->isPlaceholder())
                throw string("SpriteCreator::getFrameBitmap: Image is a Placeholder.");

            unsigned long ilXDelta;
            unsigned long ilYDelta;
            if(opSprite->getDivisionDirection() == gametools::sprite::Sprite::RIGHTWARD) {
                ilXDelta = opSprite->getDivisionOrigin().x + (ilDivisionInx * opSprite->getDivisionWidth());
                ilYDelta = opSprite->getDivisionOrigin().y;
            } else if(opSprite->getDivisionDirection() == gametools::sprite::Sprite::LEFTWARD) {
                ilXDelta = opSprite->getDivisionOrigin().x + ((opSprite->getDivisionCount() - ilDivisionInx - 1) * opSprite->getDivisionWidth());
                ilYDelta = opSprite->getDivisionOrigin().y;
            } else if(opSprite->getDivisionDirection() == gametools::sprite::Sprite::DOWNWARD) {
                ilXDelta = opSprite->getDivisionOrigin().x;
                ilYDelta = opSprite->getDivisionOrigin().y + ((opSprite->getDivisionCount() - ilDivisionInx - 1) * opSprite->getDivisionHeight());
            } else if(opSprite->getDivisionDirection() == gametools::sprite::Sprite::UPWARD) {
                ilXDelta = opSprite->getDivisionOrigin().x;
                ilYDelta = opSprite->getDivisionOrigin().y + (ilDivisionInx * opSprite->getDivisionHeight());
            }

            return olBitmap->getImage(ilXDelta, ilYDelta, opSprite->getDivisionWidth(), opSprite->getDivisionHeight());
        }

        shared_ptr <CONST_BORDER_TIER> SpriteCreator::getDeltaBorder(shared_ptr <Sprite> opSprite, size_t ipFrameIndex, bool bpActive, SI3DPOINT rpPosition, float fpAngle, SF2DPOINT rpScale, SI3DPOINT rpPosDelta, float fpAngleDelta, SF2DPOINT rpScaleDelta) {
            if(!opSprite)
                throw string("SpriteCreator::getDeltaBorder: Sprite cannot be null.");
            if(ipFrameIndex >= opSprite->getFrameCount())
                throw string("SpriteCreator::getDeltaBorder: Invalid Frame Index.");

            shared_ptr <BORDER_TIER> alSelectedBorder = bpActive? opSprite->getFrames()->at(ipFrameIndex).active_borders: opSprite->getFrames()->at(ipFrameIndex).passive_borders;

            if(alSelectedBorder->size() == 0)
                return make_shared <CONST_BORDER_TIER>();

            shared_ptr <BORDER_TIER> alTmp = make_shared <BORDER_TIER>();
            for(size_t i = 0; i < alSelectedBorder->size(); i++) {
                alTmp->push_back(make_shared <BORDER_REF>());
                for(size_t j = 0; j < alSelectedBorder->at(i)->size(); j++) {
                    alTmp->back()->push_back(make_shared <BORDER>());
                    for(size_t k = 0; k < alSelectedBorder->at(i)->at(j)->size(); k++) {
                        alTmp->back()->back()->push_back(utilities::getNewVector(alSelectedBorder->at(i)->at(j)->at(k).x, alSelectedBorder->at(i)->at(j)->at(k).y, alSelectedBorder->at(i)->at(j)->at(k).z, rpPosition.x, rpPosition.y, rpPosition.z, fpAngle, rpScale.x, rpScale.y));
                        if(rpPosDelta.x != 0 || rpPosDelta.y != 0 || fpAngleDelta != 0 || rpScaleDelta.x != 0 || rpScaleDelta.y != 0)
                            alTmp->back()->back()->push_back(utilities::getNewVector(alSelectedBorder->at(i)->at(j)->at(k).x, alSelectedBorder->at(i)->at(j)->at(k).y, alSelectedBorder->at(i)->at(j)->at(k).z, rpPosition.x + rpPosDelta.x, rpPosition.y + rpPosDelta.y, rpPosition.z, fpAngle + fpAngleDelta, rpScale.x + rpScaleDelta.x, rpScale.y + rpScaleDelta.y));
                    }
                }
            }

            return const_pointer_cast <CONST_BORDER_TIER> (alTmp);
        }

        std::shared_ptr <Sprite> SpriteCreator::create(
            size_t hpImage,
            UI3DPOINT& rpSourcePosition,
            unsigned long ipDivisionWidth,
            unsigned long ipDivisionHeight,
            unsigned char ipDirection,
            unsigned long ipDivisionCount,
            bool bpReverse,
            shared_ptr <vector <Sprite::FrameDescriptor>> apFrames,
            unsigned char ipSearchMode,
            unsigned char ipActiveBorderType,
            unsigned char ipPassiveBorderType,
            const RGBA& ipBackgroundColor,
            unsigned char ipValidAlpha,
            unsigned char ipSegmentLength,
            unsigned int ipBorderSize,
            unsigned int ipBackTrace
        ) {
            std::shared_ptr <BORDER_DIVISION> alActiveBorders = EMPTY_BORDER;
            if(ipActiveBorderType != BORDER_NULL) {
                alActiveBorders = findBorders(hpImage, rpSourcePosition, ipDivisionWidth, ipDivisionHeight, ipDirection, ipDivisionCount, ipSearchMode, ipActiveBorderType, ipBackgroundColor, ipValidAlpha, ipSegmentLength, ipBorderSize, ipBackTrace);
                for(size_t i = 0; i < apFrames->size(); i++) {
                    if(apFrames->at(i).division_inx >= ipDivisionCount)
                        throw string("SpriteCreator::create: Division Index out of Bounds for a Frame.");
                    apFrames->at(i).active_borders = alActiveBorders->at(apFrames->at(i).division_inx);
                }
            }

            std::shared_ptr <BORDER_DIVISION> alPassiveBorders = EMPTY_BORDER;
            if(ipPassiveBorderType != BORDER_NULL) {
                alPassiveBorders = findBorders(hpImage, rpSourcePosition, ipDivisionWidth, ipDivisionHeight, ipDirection, ipDivisionCount, ipSearchMode, ipPassiveBorderType, ipBackgroundColor, ipValidAlpha, ipSegmentLength, ipBorderSize, ipBackTrace);
                for(size_t i = 0; i < apFrames->size(); i++) {
                    if(apFrames->at(i).division_inx >= ipDivisionCount)
                        throw string("SpriteCreator::create: Division Index out of Bounds for a Frame.");
                    apFrames->at(i).passive_borders = alPassiveBorders->at(apFrames->at(i).division_inx);
                }
            }

            return make_shared <Sprite> (hpImage, rpSourcePosition, ipDivisionWidth, ipDivisionHeight, ipDirection, ipDivisionCount, apFrames, ipBackgroundColor);
        }

        shared_ptr <Sprite> SpriteCreator::create(
            size_t hpImage,
            bool bpReverse,
            shared_ptr <vector <Sprite::FrameDescriptor>> apFrames,
            unsigned char ipSearchMode,
            unsigned char ipActiveBorderType,
            unsigned char ipPassiveBorderType,
            const tools::image::RGBA& ipBackgroundColor,
            unsigned char ipValidAlpha,
            unsigned char ipSegmentLength,
            unsigned int ipBorderSize,
            unsigned int ipBackTrace
        ) {
            shared_ptr <RGBAImage> olTmpImage = gametools::container::ResourceManager::getInstance()->getBitmap(hpImage);
            unsigned long ilDivisionHeight;
            unsigned long ilDivisionWidth;
            unsigned long ilDivisionCount;
            unsigned char ilDirection;

            if(olTmpImage->getHeight() <= olTmpImage->getWidth()) {
                ilDivisionHeight = olTmpImage->getHeight();
                ilDivisionWidth = olTmpImage->getHeight();
                if(olTmpImage->getWidth() % olTmpImage->getHeight() != 0)
                    throw string("AlphaSprite::AlphaSprite: Cannot extract Frames from Image, Source cannot be divided in Equal Parts.");
                ilDivisionCount = olTmpImage->getWidth() / olTmpImage->getHeight();
                ilDirection = Sprite::RIGHTWARD;
            } else if(olTmpImage->getHeight() > olTmpImage->getWidth()) {
                ilDivisionHeight = olTmpImage->getWidth();
                ilDivisionWidth = olTmpImage->getWidth();
                if(olTmpImage->getHeight() % olTmpImage->getWidth() != 0)
                    throw string("AlphaSprite::AlphaSprite: Cannot extract Frames from Image, Source cannot be divided in Equal Parts.");
                ilDivisionCount = olTmpImage->getHeight() / olTmpImage->getWidth();
                ilDirection = Sprite::DOWNWARD;
            }

            UI3DPOINT rlTmpPos = {0, 0, 0};
            return create(
                hpImage,
                rlTmpPos,
                ilDivisionWidth,
                ilDivisionHeight,
                ilDirection,
                ilDivisionCount,
                bpReverse,
                apFrames,
                ipSearchMode,
                ipActiveBorderType,
                ipPassiveBorderType,
                ipBackgroundColor,
                ipValidAlpha,
                ipSegmentLength,
                ipBorderSize,
                ipBackTrace
            );
        }

        shared_ptr <BORDER_DIVISION> SpriteCreator::findBorders(
            size_t hpImage,
            UI3DPOINT& rpSourcePosition,
            unsigned long ipDivisionWidth,
            unsigned long ipDivisionHeight,
            unsigned char ipDirection,
            unsigned long ipDivisionCount,
            unsigned char ipSearchMode,
            unsigned char ipBorderType,
            const RGBA& ipBackgroundColor,
            unsigned char ipValidAlpha,
            unsigned char ipSegmentLength,
            unsigned int ipBorderSize,
            unsigned int ipBackTrace
        ) {
            if(ipDirection == Sprite::REVERSE)
                throw string("SpriteCreator::findBorders: Cannot reverse.");

            if(ipDivisionCount == 0)
                throw string("SpriteCreator::findBorders: Sprite must have at least 1 Frame.");

            shared_ptr <RGBAImage> olTmpImage = gametools::container::ResourceManager::getInstance()->getBitmap(hpImage);
            if(olTmpImage->isPlaceholder())
                throw string("SpriteCreator::findBorders: Bitmap is a Placeholder.");

            if(rpSourcePosition.x >= olTmpImage->getWidth())
                throw string("SpriteCreator::findBorders: X Coordinate cannot be greater than the Width of the Sprite.");

            if((ipDirection == Sprite::RIGHTWARD || ipDirection == Sprite::LEFTWARD) && rpSourcePosition.x + (ipDivisionWidth * ipDivisionCount) - 1 >= olTmpImage->getWidth())
                throw string("SpriteCreator::findBorders: Frames do not fit into the Space determined by the Frame Width and the Frame Count from the X Source.");

            if(rpSourcePosition.y >= olTmpImage->getHeight())
                throw string("SpriteCreator::findBorders: Y Coordinate cannot be greater than the Height of the Sprite.");

            if((ipDirection == Sprite::UPWARD || ipDirection == Sprite::DOWNWARD) && rpSourcePosition.y + (ipDivisionHeight * ipDivisionCount) - 1 >= olTmpImage->getHeight())
                throw string("SpriteCreator::findBorders: Frames do not fit into the Space determined by the Frame Height and the Frame Count from the Y Source.");

            if(ipSearchMode != BY_ALPHA && ipSearchMode != BY_COLOR)
                throw string("SpriteCreator::findBorders: Find Mode not exists.");

            shared_ptr <BORDER_DIVISION> alBorders = make_shared <BORDER_DIVISION>();
            if(ipBorderType == BORDER_NULL)
                return alBorders;

            if(ipSearchMode != BY_ALPHA && ipSearchMode != BY_COLOR)
                throw string("SpriteCreator::findBorders: Mode not defined.");

            if(ipBorderType > (BORDER_BOX | BORDER_QUADRANT | BORDER_LOOSE_CONTOUR | BORDER_TIGHT_CONTOUR))
                throw string("SpriteCreator::findBorders: Type of Border not defined.");

            unsigned long ilXDelta;
            unsigned long ilYDelta;
            for(unsigned int frame = 0; frame < ipDivisionCount; frame++) {
                if(ipDirection == Sprite::RIGHTWARD) {
                    ilXDelta = rpSourcePosition.x + (frame * ipDivisionWidth);
                    ilYDelta = rpSourcePosition.y;
                } else if(ipDirection == Sprite::LEFTWARD) {
                    ilXDelta = rpSourcePosition.x + ((ipDivisionCount - frame - 1) * ipDivisionWidth);
                    ilYDelta = rpSourcePosition.y;
                } else if(ipDirection == Sprite::DOWNWARD) {
                    ilXDelta = rpSourcePosition.x;
                    ilYDelta = rpSourcePosition.y + ((ipDivisionCount - frame - 1) * ipDivisionHeight);
                } else if(ipDirection == Sprite::UPWARD) {
                    ilXDelta = rpSourcePosition.x;
                    ilYDelta = rpSourcePosition.y + (frame * ipDivisionHeight);
                }
                const shared_ptr <const RGBAImage> plTmpFrame = olTmpImage->getImage(ilXDelta, ilYDelta, ipDivisionWidth, ipDivisionHeight);
                alBorders->push_back(make_shared <BORDER_TIER>());
                if(ipBorderType & BORDER_BOX) { // Esta es la Fontera más externa, por lo tanto debe estar en el Primer Nivel.
                    alBorders->back()->push_back(make_shared <BORDER_REF> ());
                    alBorders->back()->back()->push_back((ipSearchMode == BY_ALPHA ? gametools::utilities::getImageBoxBorderByAlpha(plTmpFrame, ipValidAlpha) : gametools::utilities::getImageBoxBorderByColor(plTmpFrame, ipBackgroundColor))[0]);
                }
                if(ipBorderType & BORDER_QUADRANT) { // Esta es la Primera Frontera contenida en BOXBORDER, está en el Nivel Siguiente.
                    BORDER_REF alNewBorders = ipSearchMode == BY_ALPHA ? gametools::utilities::getImageBoxBorderByAlpha(plTmpFrame, ipValidAlpha, true) : gametools::utilities::getImageBoxBorderByColor(plTmpFrame, ipBackgroundColor, true);
                    alBorders->back()->push_back(make_shared <BORDER_REF> ());
                    for(size_t i = 0; i < alNewBorders.size(); i++)
                        alBorders->back()->back()->push_back(alNewBorders[i]);
                }
                if(ipBorderType & BORDER_LOOSE_CONTOUR) { // Esta es la Frontera Anterior a la Frontera más Interna, está en el Penúltimo Nivel.
                    alBorders->back()->push_back(make_shared <BORDER_REF>());
                    alBorders->back()->back()->push_back(ipSearchMode == BY_ALPHA ? gametools::utilities::getImageLooseContourBorderByAlpha(plTmpFrame, ipValidAlpha) : gametools::utilities::getImageLooseContourBorderByColor(plTmpFrame, ipBackgroundColor));
                }
                if(ipBorderType & BORDER_TIGHT_CONTOUR) { // Esta es la Frontera más Interna, está en el Último Nivel.
                    alBorders->back()->push_back(make_shared <BORDER_REF> ());
                    alBorders->back()->back()->push_back(ipSearchMode == BY_ALPHA ? gametools::utilities::getImageTightContourBorderByAlpha(plTmpFrame, ipValidAlpha, ipSegmentLength, ipBorderSize, ipBackTrace) : gametools::utilities::getImageTightContourBorderByColor(plTmpFrame, ipBackgroundColor, ipSegmentLength, ipBorderSize, ipBackTrace));
                }
            }

            unsigned long ilXCorr = ipDivisionWidth / 2;
            unsigned long ilYCorr = ipDivisionHeight / 2;
            for(auto it1 = alBorders->begin(); it1 != alBorders->end(); it1++)
                for(auto it2 = (*it1)->begin(); it2 != (*it1)->end(); it2++)
                    for(auto it3 = (*it2)->begin(); it3 != (*it2)->end(); it3++)
                        for(auto it4 = (*it3)->begin(); it4 != (*it3)->end(); it4++) {
                            it4->x -= ilXCorr;
                            it4->y -= ilYCorr;
                        }
                            

            return alBorders;
        }

        //////////////////////////////////////////////////////////////////////////////////////

    } // END SPRITE

    //////////////////////////////////////////////////////////////////////////////////////
    // CONTAINERS

    namespace container {

        bool NULL_TILE_BEHAVIOUR(shared_ptr <TileInstance> opInstance, shared_ptr <const EVENT> opEvent) {
            return false;
        }

        bool NULL_LEVEL_BEHAVIOUR(shared_ptr <Level> opLevel, shared_ptr <const EVENT> opEvent) {
            return false;
        }

        TileState::TileState() {
            igAltId = UNINITIALIZED;
            bgIsLoaded = false;
            pgFunction = NULL_TILE_BEHAVIOUR;
            agAudios = make_shared <vector <size_t>> ();
        }

        TileState::TileState(
            std::shared_ptr <std::vector <std::shared_ptr <sprite::Sprite>>> apSprites,
            std::shared_ptr <std::vector <size_t>> apAudios,
            function <bool (shared_ptr <TileInstance> opInstance, shared_ptr <const EVENT> opEvent)> ppFunction
        ) {
            if(!apSprites)
                throw string("TileState::TileState: Sprites Vector cannot be null.");
            if(!apAudios)
                throw string("TileState::TileState: Audios Vector cannot be null.");
            if(!ppFunction)
                throw string("TileState::TileState: Behaviour Function cannot be null.");

            igAltId = UNINITIALIZED;
            bgIsLoaded = false;
            for(size_t i = 1; i < apSprites->size(); i++)
                if(apSprites->at(i - 1)->getDivisionHeight() != apSprites->at(i)->getDivisionHeight() || apSprites->at(i - 1)->getDivisionWidth() != apSprites->at(i)->getDivisionWidth())
                    throw string("TileState::TileState: All Sprites must have the Same Dimesions.");

            agSprites = *apSprites;
            agAudios = apAudios;
            pgFunction = ppFunction;
        }

        void TileState::setAlternativeStateId(size_t ipId) {
            igAltId = ipId;
        }

        size_t TileState::getAlternativeStateId() const {
            return igAltId;
        }

        unsigned int TileState::getWidth() const {
            return agSprites[0]->getDivisionWidth();
        }

        unsigned int TileState::getHeight() const {
            return agSprites[0]->getDivisionHeight();
        }

        size_t TileState::getSpriteCount() const {
            return agSprites.size();
        }

        size_t TileState::getAudioCount() const {
            return agAudios->size();
        }

        bool TileState::isLoaded() const {
            return bgIsLoaded;
        }

        bool TileState::catchEvent(shared_ptr <TileInstance> opInstance, shared_ptr <const EVENT> opEvent) const {
            if(!opInstance || !opEvent)
                return false;
            return pgFunction(opInstance, opEvent);
        }

        // The first level corresponds to the sprites of the tile, and the second level are the borders of the ipTier level of the current frame of each sprite.
        shared_ptr <CONST_BORDER_REF> TileState::getFrameBorder(size_t ipSpriteInx, size_t ipFrameInx, size_t ipTier, bool bpActive, SI3DPOINT rpPosition, float fpAngle, SF2DPOINT rpScale, SI3DPOINT rpPosDelta, float fpAngleDelta, SF2DPOINT rpScaleDelta) const {
            if(ipSpriteInx >= agSprites.size())
                throw string("TileState::getFrameBorder: Sprite Index out of Bounds.");

            shared_ptr <BORDER_REF> plResult = make_shared <BORDER_REF> ();

            const shared_ptr <CONST_BORDER_TIER> alTmpTier = gametools::sprite::SpriteCreator::getDeltaBorder(agSprites[ipSpriteInx], ipFrameInx, bpActive, rpPosition, fpAngle, rpScale, rpPosDelta, fpAngleDelta, rpScaleDelta);
            if(ipTier < alTmpTier->size())
                plResult = alTmpTier->at(ipTier);

            return plResult;
        }

        const shared_ptr <const sprite::Sprite> TileState::getSprite(size_t ipSpriteInx) const {
            if(ipSpriteInx >= agSprites.size())
                throw string("TileState::getSprite: Sprite Index out of Bounds.");
            return agSprites[ipSpriteInx];
        }

        const shared_ptr <const tools::audio::Audio> TileState::getAudio(size_t ipAudioInx) const {
            if(!bgIsLoaded)
                throw string("TileState::getAudio: Resources not loaded.");
            if(ipAudioInx >= agAudios->size())
                throw string("TileState::getAudio: Audio Index out of Bounds.");
            return ResourceManager::getInstance()->getAudio(agAudios->at(ipAudioInx));
        }

        void TileState::loadResources(bool bpAsynchronousLoad) {
            shared_ptr <ResourceManager> olManager = ResourceManager::getInstance();
            shared_ptr <vector <size_t>> alIndexes = make_shared <vector <size_t>> ();
            for(size_t i = 0; i < agSprites.size(); i++)
                alIndexes->push_back(agSprites[i]->getBitmapHandle());
            olManager->loadBitmaps(alIndexes);
            olManager->loadAudios(agAudios);
            if(!bpAsynchronousLoad)
                waitResourcesLoad();
        }

        void TileState::unloadResources(bool bpUnloadAudio) {
            shared_ptr <ResourceManager> olManager = ResourceManager::getInstance();
            shared_ptr <vector <size_t>> alIndexes = make_shared <vector <size_t>>();
            for(size_t i = 0; i < agSprites.size(); i++)
                alIndexes->push_back(agSprites[i]->getBitmapHandle());
            olManager->unloadBitmaps(alIndexes);
            if(bpUnloadAudio)
                olManager->unloadAudios(agAudios);
            bgIsLoaded = false;
        }

        bool TileState::isLoadingAudio() {
            shared_ptr <ResourceManager> olManager = ResourceManager::getInstance();
            if(olManager->isLoadingBitmaps() || olManager->isLoadingAudios())
                return true;

            waitResourcesLoad();
            return false;
        }

        void TileState::waitResourcesLoad() {
            if(bgIsLoaded)
                return;

            shared_ptr <ResourceManager> olManager = ResourceManager::getInstance();
            olManager->waitBitmapsLoad();
            olManager->waitAudiosLoad();

            bgIsLoaded = true;
        }

        //////////////////////////////////////////////////////////////////////////////////////

        size_t TileClass::getLastClassId() {
            static size_t igLastClassKey = 0;
            return igLastClassKey++;
        }

        TileClass::TileClass() {
            bgIsLoaded = false;
            igId = getLastClassId();
            igAltId = UNINITIALIZED;
            igStartStateIndex = MAINSTATEINDEX;
            pgFunction = NULL_TILE_BEHAVIOUR;
        }

        TileClass::TileClass(
            shared_ptr <vector <shared_ptr <TileState>>> apStates,
            size_t ipStartState,
            function <bool (shared_ptr <TileInstance> opInstance, shared_ptr <const EVENT> opEvent)> ppFunction
        ) {
            if(!apStates)
                throw string("TileClass::TileClass: States Vector cannot be null.");
            if(!ppFunction)
                throw string("TileClass::TileClass: Behaviour Function cannot be null.");

            bgIsLoaded = false;
            igId = getLastClassId();
            igAltId = UNINITIALIZED;
            igStartStateIndex = ipStartState;
            agStates = *apStates;
            pgFunction = ppFunction;
        }

        bool TileClass::catchEvent(shared_ptr <TileInstance> opInstance, shared_ptr <const EVENT> opEvent) const {
            if(!opEvent)
                return false;
            return pgFunction(opInstance, opEvent);
        }

        bool TileClass::isLoaded() const {
            return bgIsLoaded;
        }

        void TileClass::loadResources(bool bpAsynchronousLoad) {
            for(size_t i = 0; i < agStates.size(); i++)
                agStates[i]->loadResources(bpAsynchronousLoad);
            if(bpAsynchronousLoad)
                bgIsLoaded = true;
        }

        void TileClass::unloadResources(bool bpUnloadAudios) {
            for(size_t i = 0; i < agStates.size(); i++)
                agStates[i]->unloadResources(bpUnloadAudios);
            bgIsLoaded = false;
        }

        bool TileClass::isLoadingAudio() {
            for(size_t i = 0; i < agStates.size(); i++)
                if(agStates[i]->isLoadingAudio())
                    return true;

            return false;
        }

        void TileClass::waitResourcesLoad() {
            for(size_t i = 0; i < agStates.size(); i++)
                agStates[i]->waitResourcesLoad();
            bgIsLoaded = true;
        }

        size_t TileClass::getClassId() const {
            return igId;
        }

        size_t TileClass::getStartStateIndex() const {
            return igStartStateIndex;
        }

        size_t TileClass::getStatesCount() const {
            return agStates.size();
        }

        const shared_ptr <const TileState> TileClass::getState(size_t ipStateInx) const {
            if(ipStateInx >= agStates.size())
                throw string("TileClass::getState: Index out of Range.");
            return agStates[ipStateInx];
        }

        void TileClass::setAlternativeClassId(size_t ipId) {
            igAltId = ipId;
        }

        size_t TileClass::getAlternativeClassId() const {
            return igAltId;
        }

        //////////////////////////////////////////////////////////////////////////////////////
        
        const TileInstance::SpriteControlDescriptor TileInstance::NULL_SPRITE_CONTROL = {false, false, false, false, false, 0, sprite::Sprite::NOLOOP, 0, 0, chrono::milliseconds(0), {0, 0, 0}, 0, {1, 1}, shared_ptr <std::vector <gametools::sprite::Sprite::FrameDescriptor>> (nullptr)};

        void TileInstance::initialize() {
            bgIsInitialized = false;
            igId = UNINITIALIZED;
            igAltId = UNINITIALIZED;
            bgResetDelta = false;
            igActiveStateIndex = -1;
            pgLevel = make_shared <Level>();
            igXPosType = POS_PERCENTAGE;
            igYPosType = POS_PERCENTAGE;
            igAngleType = POS_PERCENTAGE;
            igXScaleType = POS_PERCENTAGE;
            igYScaleType = POS_PERCENTAGE;
            rgPositionModifier = {PERCENT_100, PERCENT_100, PERCENT_100};
            fgAngleModifier = 1;
            rgScaleModifier = {1, 1};
            cgCollisionType = COLLISION_INTERCEPTION;
            rgPositionDelta = sprite::DEFAULT_POS_DELTA;
            fgAngleDelta = sprite::DEFAULT_ANGLE_DELTA;
            rgScaleDelta = sprite::DEFAULT_SCALE_DELTA;
            agPlayers = vector <vector <shared_ptr <AsynchronousPlayer>>>();
            bgIsAudioLoaded = false;
            igStateChangeCounter = 0;
        }

        TileInstance::TileInstance() {
            initialize();
            rgPosition = {0, 0, 0};
            fgAngle = 0;
            rgScale = {1, 1};
            pgClass = make_shared <TileClass> ();
            pgSpriteControl = vector <vector <SpriteControlDescriptor>> ();
            agAudioCommands = vector <vector <AudioCommand>> ();
            bgIsInitialized = false;
        }

        TileInstance::TileInstance(SI3DPOINT rpPosition, shared_ptr <TileClass> ppClass, shared_ptr <renderer::Renderer> opRenderer, float fpAngle, SF2DPOINT rpScale) {
            if(!ppClass)
                throw string("TileInstance::TileInstance: Class cannot be null.");
            if(ppClass->getClassId() == TileClass::UNINITIALIZED)
                throw string("TileInstance::TileInstance: Uninitialized Class.");
            if(!opRenderer)
                throw string("TileInstance::TileInstance: Renderer cannot be null.");

            initialize();
            rgPosition = rpPosition;
            fgAngle = fpAngle;
            rgScale = rpScale;
            pgClass = ppClass;
            ogRenderer = opRenderer;
            pgSpriteControl = vector <vector <SpriteControlDescriptor>> (pgClass->getStatesCount());
            for(size_t i = 0; i < pgSpriteControl.size(); i++)
                pgSpriteControl[i] = vector <SpriteControlDescriptor> (pgClass->getState(i)->getSpriteCount(), NULL_SPRITE_CONTROL);
            agAudioCommands = vector <vector <AudioCommand>> ();
            for(size_t i = 0; i < pgClass->getStatesCount(); i++)
                agAudioCommands.push_back(vector <AudioCommand> (pgClass->getState(i)->getAudioCount(), {AudioCommand::NO_COMMAND_SOUND, false, 0}));
            setActiveStateIndex(pgClass->getStartStateIndex());
            bgIsInitialized = true;
        }

        bool TileInstance::throwEvent(shared_ptr <const EVENT> opEvent) {
            return pgLevel->throwEvent(opEvent);
        }

        bool TileInstance::catchEvent(shared_ptr <const EVENT> opEvent) {
            bool blResult = pgClass->catchEvent(const_pointer_cast <TileInstance> (shared_from_this()), opEvent);
            if(blResult && getInstanceId() != TileInstance::UNINITIALIZED) {
                // This cycle allows to chain state changes that can occur during the event processing.
                size_t ilActiveStateIndex = getActiveStateIndex();
                while((blResult = pgClass->getState(igActiveStateIndex)->catchEvent(const_pointer_cast <TileInstance> (shared_from_this()), opEvent)) && getActiveStateIndex() != ilActiveStateIndex)
                    ilActiveStateIndex = getActiveStateIndex();
            }
            return blResult;
        }

        bool TileInstance::isAudioLoaded() const {
            return bgIsAudioLoaded;
        }

        void TileInstance::loadAudios() {
            if(bgIsAudioLoaded)
                return;
            if(!pgClass->isLoaded())
                throw string("TileInstance::loadAudios: Resources of Class are not loaded.");

            for(size_t i = 0; i < pgClass->getStatesCount(); i++) {
                agPlayers.push_back(vector <shared_ptr <AsynchronousPlayer>>());
                for(size_t j = 0; j < pgClass->getState(i)->getAudioCount(); j++)
                    agPlayers[i].push_back(make_shared <AsynchronousPlayer> (pgClass->getState(i)->getAudio(j)));
            }

            bgIsAudioLoaded = true;
        }

        void TileInstance::unloadAudios() {
            agPlayers.clear();
            bgIsAudioLoaded = false;
        }

        bool TileInstance::isLoadingAudio() {
            if(pgClass->isLoadingAudio())
                return true;

            loadAudios();
            return false;
        }

        bool TileInstance::isTimeUp(size_t ipSpriteInx, chrono::milliseconds tpReferenceTime) {
            bool blResult = false;
            SpriteControlDescriptor& rlDescriptor = pgSpriteControl[igActiveStateIndex][ipSpriteInx];

            if(tpReferenceTime >= rlDescriptor.next_tick_time) { // Checks if the time for processing the current frame is already up in the corresponding sprite.
                const gametools::sprite::Sprite::FrameDescriptor& rlFrame = rlDescriptor.buffer->at(rlDescriptor.frame_inx);
                if(rlDescriptor.tick_accumulator < rlFrame.tick_count)
                    rlDescriptor.tick_accumulator++;
                else {
                    rlDescriptor.tick_accumulator = 0;
                    blResult = true;
                }
                rlDescriptor.next_tick_time = tpReferenceTime + rlFrame.tick_length;
            }
            return blResult;
        }

        sprite::Sprite::FrameDescriptor TileInstance::getNextFrame(size_t ipSpriteInx) {
            SpriteControlDescriptor& rlDescriptor = pgSpriteControl[igActiveStateIndex][ipSpriteInx];
            sprite::Sprite::FrameDescriptor rlFrame = pgSpriteControl[igActiveStateIndex][ipSpriteInx].buffer->at(rlDescriptor.frame_inx);

            if(getStateChangeCount() != 0)
                return rlFrame;

            bool blDoLoop = false;
            if(rlFrame.marker & sprite::Sprite::MARK_NEXT)
                rlDescriptor.loop_index = rlDescriptor.frame_inx;
            if(rlFrame.marker & sprite::Sprite::MARK_FOR) {
                if(rlFrame.param == sprite::Sprite::ALWAYSLOOP) {
                    rlDescriptor.frame_inx = rlDescriptor.loop_index;
                    blDoLoop = true;
                } else if(rlFrame.param != sprite::Sprite::NOLOOP && rlDescriptor.loop_counter == sprite::Sprite::NOLOOP) {
                    rlDescriptor.loop_counter = rlFrame.param - 1;
                    rlDescriptor.frame_inx = rlDescriptor.loop_index;
                    blDoLoop = true;
                } else if(rlDescriptor.loop_counter != sprite::Sprite::NOLOOP && rlDescriptor.loop_counter > 0) {
                    rlDescriptor.loop_counter--;
                    rlDescriptor.frame_inx = rlDescriptor.loop_index;
                    blDoLoop = true;
                } else
                    pgSpriteControl[igActiveStateIndex][ipSpriteInx].loop_counter = sprite::Sprite::NOLOOP;
            }
            if(!blDoLoop) {
                if(rlFrame.marker & sprite::Sprite::MARK_STOP) {
                    rlDescriptor.finished = true;
                    rlDescriptor.stop = true;
                } else if(rlFrame.marker & sprite::Sprite::MARK_JUMP)
                    rlDescriptor.frame_inx = rlFrame.param;
                else {
                    rlDescriptor.finished = (rlDescriptor.reverse && rlDescriptor.frame_inx == 0) || (!rlDescriptor.reverse && rlDescriptor.frame_inx == rlDescriptor.buffer->size() - 1);
                    if(rlFrame.marker & sprite::Sprite::MARK_FORWARD)
                        rlDescriptor.reverse = false;
                    else if(rlFrame.marker & sprite::Sprite::MARK_BACKWARD)
                        rlDescriptor.reverse = true;

                    if((rlDescriptor.reverse && rlDescriptor.frame_inx > 0) || (!rlDescriptor.reverse && rlDescriptor.frame_inx < rlDescriptor.buffer->size() - 1)) // Manages the progression of the sprite animation.
                        rlDescriptor.frame_inx += rlDescriptor.reverse ? -1 : 1;
                }
            }

            return rlDescriptor.buffer->at(rlDescriptor.frame_inx);
        }

        void TileInstance::playInstance(bool bpDrawBorder, bool bpActive, unsigned char ipPixelSize, RGBA rlBorderColor) {
            if(!bgIsInitialized)
                throw string("TileInstance::renderTileInstance: Tile not initialized.");
            if(!bgIsAudioLoaded)
                throw string("TileInstance::renderTileInstance: Resources not loaded.");

            const SCREEN olScreen = pgLevel->getScreen();
            shared_ptr <TileState> olState = const_pointer_cast <TileState> (pgClass->getState(igActiveStateIndex));
            chrono::milliseconds ilActualTime = chrono::duration_cast <chrono::milliseconds> (chrono::system_clock::now().time_since_epoch()); // Current time used by the reckoning.
            map<size_t, list<size_t>, greater<size_t>> rlZBuffer;
            float flTmpAngle = 0;
            SI3DPOINT rlTmpPosition;
            SF2DPOINT rlTmpScale;
            for(size_t ilSpriteInx = 0; ilSpriteInx < olState->getSpriteCount(); ilSpriteInx++) {
                if(ilSpriteInx == TileState::MAINSPRITEINDEX || pgSpriteControl[igActiveStateIndex][ilSpriteInx].active) {
                    if(getStateChangeCount() != 0 || isTimeUp(ilSpriteInx, ilActualTime)) {
                        const sprite::Sprite::FrameDescriptor& olFrame = getNextFrame(ilSpriteInx);
                        if(!pgSpriteControl[igActiveStateIndex][ilSpriteInx].stop) {
                            if(ilSpriteInx == TileState::MAINSPRITEINDEX) { // Updates the position of the instance using the position modifier (rgPositionModifier) according to the current frame of the first sprite on processing.
                                if(igXPosType == POS_ABSOLUTE) {
                                    rgPositionDelta.x = bgResetDelta ? 0 : rgPosition.x - rgPositionModifier.x;
                                    rgPosition.x = rgPositionModifier.x;
                                } else if(igXPosType == POS_DELTA) {
                                    rgPositionDelta.x = bgResetDelta ? 0 : -rgPositionModifier.x;
                                    rgPosition.x += rgPositionModifier.x;
                                } else if(igXPosType == POS_PERCENTAGE && rgPositionModifier.x != PERCENT_0) {
                                    if(rgPositionModifier.x != PERCENT_100) {
                                        // It is not necessary to multiply rgPositionModifier.x by 100 since It should be formatted (RE: E+DD; E: Integer; D: Decimal).
                                        long result = ((olFrame.delta_pos.x * 100) * rgPositionModifier.x) / 1000000;
                                        rgPositionDelta.x = bgResetDelta ? 0 : -result;
                                        rgPosition.x += result;
                                    } else {
                                        rgPositionDelta.x = bgResetDelta ? 0 : -olFrame.delta_pos.x;
                                        rgPosition.x += olFrame.delta_pos.x;
                                    }
                                }
                                if(igYPosType == POS_ABSOLUTE) {
                                    rgPositionDelta.y = bgResetDelta ? 0 : rgPosition.y - rgPositionModifier.y;
                                    rgPosition.y = rgPositionModifier.y;
                                } else if(igYPosType == POS_DELTA) {
                                    rgPositionDelta.y = bgResetDelta ? 0 : -rgPositionModifier.y;
                                    rgPosition.y += rgPositionModifier.y;
                                } else if(igYPosType == POS_PERCENTAGE && rgPositionModifier.y != PERCENT_0) {
                                    if(rgPositionModifier.y != PERCENT_100) {
                                        // It is not necessary to multiply rgPositionModifier.x by 100 since It should be formatted (RE: E+DD; E: Integer; D: Decimal).
                                        long result = ((olFrame.delta_pos.y * 100) * rgPositionModifier.y) / 1000000;
                                        rgPositionDelta.y = bgResetDelta ? 0 : -result;
                                        rgPosition.y += result;
                                    } else {
                                        rgPositionDelta.y = bgResetDelta ? 0 : -olFrame.delta_pos.y;
                                        rgPosition.y += olFrame.delta_pos.y;
                                    }
                                }
                                igXPosType = POS_PERCENTAGE;
                                igYPosType = POS_PERCENTAGE;
                                rlTmpPosition = rgPosition;
                                if(igAngleType == POS_ABSOLUTE) {
                                    fgAngleDelta = bgResetDelta ? 0 : fgAngle - fgAngleModifier;
                                    fgAngle = fgAngleModifier;
                                } else if(igAngleType == POS_DELTA) {
                                    fgAngleDelta = bgResetDelta ? 0 : -fgAngleModifier;
                                    fgAngle += fgAngleModifier;
                                } else if(igAngleType == POS_PERCENTAGE && fgAngleModifier != 0) {
                                    if(fgAngleModifier != 1) {
                                        float result = olFrame.delta_angle * fgAngleModifier;
                                        fgAngleDelta = bgResetDelta ? 0 : -result;
                                        fgAngle += result;
                                    } else {
                                        fgAngleDelta = bgResetDelta ? 0 : -olFrame.delta_angle;
                                        fgAngle += olFrame.delta_angle;
                                    }
                                }
                                igAngleType = POS_PERCENTAGE;
                                flTmpAngle = fgAngle;
                                if(igXScaleType == POS_ABSOLUTE) {
                                    rgScaleDelta.x = bgResetDelta ? 0 : rgScale.x - rgScaleModifier.x;
                                    rgScale.x = rgScaleModifier.x;
                                } else if(igXScaleType == POS_DELTA) {
                                    rgScaleDelta.x = bgResetDelta ? 0 : -rgScaleModifier.x;
                                    rgScale.x += rgScaleModifier.x;
                                } else if(igXScaleType == POS_PERCENTAGE && rgScaleModifier.x != 0) {
                                    if(rgScaleModifier.x != 1) {
                                        float result = olFrame.delta_scale.x * rgScaleModifier.x;
                                        rgScaleDelta.x = bgResetDelta ? 0 : -result;
                                        rgScale.x += result;
                                    } else {
                                        rgScaleDelta.x = bgResetDelta ? 0 : -olFrame.delta_scale.x;
                                        rgScale.x += olFrame.delta_scale.x;
                                    }
                                }
                                if(igYScaleType == POS_ABSOLUTE) {
                                    rgScaleDelta.y = bgResetDelta ? 0 : rgScale.y - rgScaleModifier.y;
                                    rgScale.y = rgScaleModifier.y;
                                } else if(igYScaleType == POS_DELTA) {
                                    rgScaleDelta.y = bgResetDelta ? 0 : -rgScaleModifier.y;
                                    rgScale.y += rgScaleModifier.y;
                                } else if(igYScaleType == POS_PERCENTAGE && rgScaleModifier.y != 0) {
                                    if(rgScaleModifier.y != 1) {
                                        float result = olFrame.delta_scale.y * rgScaleModifier.y;
                                        rgScaleDelta.y = bgResetDelta ? 0 : -result;
                                        rgScale.y += result;
                                    } else {
                                        rgScaleDelta.y = bgResetDelta ? 0 : -olFrame.delta_scale.y;
                                        rgScale.y += olFrame.delta_scale.y;
                                    }
                                }
                                igXScaleType = POS_PERCENTAGE;
                                igYScaleType = POS_PERCENTAGE;
                                rlTmpScale = rgScale;
                                bgResetDelta = false;
                            } else {
                                rlTmpPosition = {rgPosition.x + olFrame.delta_pos.x, rgPosition.y + olFrame.delta_pos.y, rgPosition.z + olFrame.delta_pos.z};
                                flTmpAngle = fgAngle + olFrame.delta_angle;
                                rlTmpScale = {rgScale.x + olFrame.delta_scale.x, rgScale.y + olFrame.delta_scale.y};
                            }

                            const shared_ptr <const sprite::Sprite> olSprite = olState->getSprite(ilSpriteInx);
                            long ilEndX = olScreen.originX + olScreen.width;
                            long ilEndY = olScreen.originY + olScreen.height;
                            pgSpriteControl[igActiveStateIndex][ilSpriteInx].pos = rlTmpPosition;
                            pgSpriteControl[igActiveStateIndex][ilSpriteInx].angle = flTmpAngle;
                            pgSpriteControl[igActiveStateIndex][ilSpriteInx].scale = rlTmpScale;
                            pgSpriteControl[igActiveStateIndex][ilSpriteInx].show = (
                                (
                                    (rlTmpPosition.x >= olScreen.originX && rlTmpPosition.x < ilEndX && rlTmpPosition.y >= olScreen.originY && rlTmpPosition.y < ilEndY) ||
                                    (
                                        (long) (rlTmpPosition.x + olSprite->getDivisionWidth()) >= olScreen.originX &&
                                        (long) (rlTmpPosition.x + olSprite->getDivisionWidth()) < ilEndX &&
                                        (long) (rlTmpPosition.y + olSprite->getDivisionHeight()) >= olScreen.originY &&
                                        (long) (rlTmpPosition.y + olSprite->getDivisionHeight()) < ilEndY
                                    )
                                )
                            );
                        }
                    }
                    rlZBuffer[pgSpriteControl[igActiveStateIndex][ilSpriteInx].pos.z].push_back(ilSpriteInx);
                }
            }

            for(map<size_t, list<size_t>>::iterator it = rlZBuffer.begin(); it != rlZBuffer.end(); it++) {
                for(list<size_t>::iterator lt = it->second.begin(); lt != it->second.end(); lt++) {
                    const shared_ptr <const sprite::Sprite> olSprite = olState->getSprite(*lt);
                    const sprite::Sprite::FrameDescriptor& olFrame = pgSpriteControl[igActiveStateIndex][*lt].buffer->at(pgSpriteControl[igActiveStateIndex][*lt].frame_inx);
                    if(pgSpriteControl[igActiveStateIndex][*lt].show) {
                        rlTmpPosition = {pgSpriteControl[igActiveStateIndex][*lt].pos.x - olScreen.originX, pgSpriteControl[igActiveStateIndex][*lt].pos.y - olScreen.originY, pgSpriteControl[igActiveStateIndex][*lt].pos.z};
                        flTmpAngle = pgSpriteControl[igActiveStateIndex][*lt].angle;
                        rlTmpScale = {pgSpriteControl[igActiveStateIndex][*lt].scale.x, pgSpriteControl[igActiveStateIndex][*lt].scale.y};
                        ogRenderer->drawSprite(rlTmpPosition, flTmpAngle, rlTmpScale, olSprite->getBitmapHandle(), olFrame.division_pos.x, olFrame.division_pos.y, olSprite->getDivisionWidth(), olSprite->getDivisionHeight());
                        if(bpDrawBorder)
                            ogRenderer->drawBorder(rlTmpPosition, flTmpAngle, rlTmpScale, bpActive ? olFrame.active_borders : olFrame.passive_borders, ipPixelSize, rlBorderColor);
                    }
                }
            }

            for(size_t i = 0; i < olState->getAudioCount(); i++) {
                AudioCommand rlCommand = agAudioCommands[igActiveStateIndex][i];
                if(rlCommand.command != AudioCommand::NO_COMMAND_SOUND) {
                    executeAudioCommand(i, rlCommand.command, rlCommand.loop, rlCommand.limit);
                    agAudioCommands[igActiveStateIndex][i].command = AudioCommand::NO_COMMAND_SOUND;
                }
            }

            igStateChangeCounter = 0;
        }

        size_t TileInstance::getInstanceId() const {
            return igId;
        }

        void TileInstance::setInstanceId(size_t ipInstanceId) {
            igId = ipInstanceId;
        }

        void TileInstance::setOwnerLevel(shared_ptr <Level> opLevel) {
            if(!opLevel)
                throw string("TileInstance::setOwnerLevel: Level cannot be null.");

            pgLevel = opLevel;
        }

        const shared_ptr <Level> TileInstance::getOwnerLevel() const {
            return pgLevel;
        }

        void TileInstance::setCollisionType(unsigned char cpType) {
            if(cpType != COLLISION_INSIDE && cpType != COLLISION_INTERCEPTION && cpType != COLLISION_VECTOR)
                throw string("TileInstance::setCollisionType: Wrong Collision Type.");
            cgCollisionType = cpType;
        }

        unsigned char TileInstance::getCollisionType() const {
            return cgCollisionType;
        }

        void TileInstance::setRenderer(shared_ptr <gametools::renderer::Renderer> opRenderer) {
            if(!opRenderer)
                throw string("TileInstance::setRenderer: Renderer cannot be null.");
            ogRenderer = opRenderer;
        }

        const shared_ptr <gametools::renderer::Renderer> TileInstance::getRenderer() const {
            return ogRenderer;
        }

        const shared_ptr <const TileClass> TileInstance::getClass() const {
            return pgClass;
        }

        const SI3DPOINT& TileInstance::getPosition() const {
            return rgPosition;
        }

        const SI3DPOINT& TileInstance::getPositionDelta() const {
            return rgPositionDelta;
        }

        const float TileInstance::getAngle() const {
            return fgAngle;
        }

        const float TileInstance::getAngleDelta() const {
            return fgAngleDelta;
        }

        const SF2DPOINT& TileInstance::getScale() const {
            return rgScale;
        }

        const SF2DPOINT& TileInstance::getScaleDelta() const {
            return rgScaleDelta;
        }

        void TileInstance::resetDelta() {
            bgResetDelta = true;
        }

        long TileInstance::getZ() const {
            return rgPosition.z;
        }

        void TileInstance::setX(long ipX, unsigned char ipPositionType) {
            if(ipPositionType != POS_ABSOLUTE && ipPositionType != POS_DELTA && ipPositionType != POS_PERCENTAGE)
                throw string("TileInstance::setX: Invalid Position Type.");
            igXPosType = ipPositionType;
            rgPositionModifier.x = ipX;
        }

        void TileInstance::setY(long ipY, unsigned char ipPositionType) {
            if(ipPositionType != POS_ABSOLUTE && ipPositionType != POS_DELTA && ipPositionType != POS_PERCENTAGE)
                throw string("TileInstance::setY: Invalid Position Type.");
            igYPosType = ipPositionType;
            rgPositionModifier.y = ipY;
        }

        void TileInstance::setZ(long ipZ) {
            rgPositionDelta.z = rgPosition.z - ipZ;
            rgPosition.z = ipZ;
        }

        void TileInstance::setAngle(float fpAngle, unsigned char ipAngleType) {
            if(ipAngleType != POS_ABSOLUTE && ipAngleType != POS_DELTA && ipAngleType != POS_PERCENTAGE)
                throw string("TileInstance::setAngle: Invalid Angle Type.");
            igAngleType = ipAngleType;
            fgAngleModifier = fpAngle;
        }

        void TileInstance::setXScale(float fpXScale, unsigned char ipXScaleType) {
            if(ipXScaleType != POS_ABSOLUTE && ipXScaleType != POS_DELTA && ipXScaleType != POS_PERCENTAGE)
                throw string("TileInstance::setXScale: Invalid X Scale Type.");
            igXScaleType = ipXScaleType;
            rgScaleModifier.x = fpXScale;
        }

        void TileInstance::setYScale(float fpYScale, unsigned char ipYScaleType) {
            if(ipYScaleType != POS_ABSOLUTE && ipYScaleType != POS_DELTA && ipYScaleType != POS_PERCENTAGE)
                throw string("TileInstance::setYScale: Invalid Y Scale Type.");
            igYScaleType = ipYScaleType;
            rgScaleModifier.y = fpYScale;
        }

        bool TileInstance::isInitialized() const {
            return bgIsInitialized;
        }

        void TileInstance::setAlternateInstanceId(size_t ipId) {
            igAltId = ipId;
        }

        size_t TileInstance::getAlternateInstanceId() const {
            return igAltId;
        }

        size_t TileInstance::createIntegerVariable() {
            agIntegerVariables.push_back(0);
            return agIntegerVariables.size() - 1;
        }

        size_t TileInstance::createStringVariable() {
            agStringVariables.push_back("");
            return agStringVariables.size() - 1;
        }

        size_t TileInstance::createFloatVariable() {
            agFloatVariables.push_back(0);
            return agFloatVariables.size() - 1;
        }

        int& TileInstance::getIntegerVariable(size_t ipIntVariableInx) {
            if(ipIntVariableInx >= agIntegerVariables.size())
                throw string("TileInstance::getIntegerVariable: Index out of range.");
            return agIntegerVariables[ipIntVariableInx];
        }

        string& TileInstance::getStringVariable(size_t ipStrVariableInx) {
            if(ipStrVariableInx >= agStringVariables.size())
                throw string("TileInstance::getStringVariable: Index out of range.");
            return agStringVariables[ipStrVariableInx];
        }

        float& TileInstance::getFloatVariable(size_t ipDblVariableInx) {
            if(ipDblVariableInx >= agFloatVariables.size())
                throw string("TileInstance::getFloatVariable: Index out of range.");
            return agFloatVariables[ipDblVariableInx];
        }

        unsigned char TileInstance::getStateChangeCount() const {
            return igStateChangeCounter;
        }

        size_t TileInstance::getActiveStateIndex() const {
            return igActiveStateIndex;
        }

        void TileInstance::setActiveStateIndex(size_t ipStateIndex, unsigned char ipAlignment) {
            if(ipStateIndex >= pgClass->getStatesCount())
                throw string("TileInstance::setActualStateIndex: State Index out of Range.");

            if(igActiveStateIndex == ipStateIndex)
                return;

            if(bgIsInitialized) {
                if(ipAlignment != 0) {
                    if((ipAlignment & ALIGN_BOTTOM_EDGE) != 0) {
                        unsigned int ilOldHeight = pgClass->getState(igActiveStateIndex)->getHeight();
                        unsigned int ilNewHeight = pgClass->getState(ipStateIndex)->getHeight();
                        if(ilOldHeight < ilNewHeight)
                            rgPosition.y += -(int) (ilNewHeight - ilOldHeight);
                        else
                            rgPosition.y += ilOldHeight - ilNewHeight;
                    } else if((ipAlignment & ALIGN_VERTICAL_CENTERED) != 0) {
                        unsigned int ilOldHeight = pgClass->getState(igActiveStateIndex)->getHeight();
                        unsigned int ilNewHeight = pgClass->getState(ipStateIndex)->getHeight();
                        if(ilOldHeight < ilNewHeight)
                            rgPosition.y += -(int) (ilNewHeight - ilOldHeight) / 2;
                        else
                            rgPosition.y += (ilOldHeight - ilNewHeight) / 2;

                    }

                    if((ipAlignment & ALIGN_RIGHT_EDGE) != 0) {
                        unsigned int ilOldWidth = pgClass->getState(igActiveStateIndex)->getWidth();
                        unsigned int ilNewWidth = pgClass->getState(ipStateIndex)->getWidth();
                        if(ilOldWidth < ilNewWidth)
                            rgPosition.x += -(int) (ilNewWidth - ilOldWidth);
                        else
                            rgPosition.x += ilOldWidth - ilNewWidth;
                    } else if((ipAlignment & ALIGN_HORIZONTAL_CENTERED) != 0) {
                        unsigned int ilOldWidth = pgClass->getState(igActiveStateIndex)->getWidth();
                        unsigned int ilNewWidth = pgClass->getState(ipStateIndex)->getWidth();
                        if(ilOldWidth < ilNewWidth)
                            rgPosition.x += -(int) (ilNewWidth - ilOldWidth) / 2;
                        else
                            rgPosition.x += (ilOldWidth - ilNewWidth) / 2;

                    }
                }

                pgSpriteControl[igActiveStateIndex].assign(pgSpriteControl[igActiveStateIndex].size(), NULL_SPRITE_CONTROL);
                agAudioCommands[igActiveStateIndex].assign(agAudioCommands[igActiveStateIndex].size(), NULL_AUDIO_COMMAND);
            }

            igActiveStateIndex = ipStateIndex;
            setSpriteActive(TileState::MAINSPRITEINDEX);
            setFrameIndex(TileState::MAINFRAMEINDEX, 0);
            igStateChangeCounter++;
        }

        bool TileInstance::isSpriteActive(size_t ipSpriteInx) const {
            if(ipSpriteInx >= pgSpriteControl[igActiveStateIndex].size())
                throw string("TileInstance::isSpriteActive: Sprite Index out of Range.");
            return pgSpriteControl[igActiveStateIndex][ipSpriteInx].active;
        }

        void TileInstance::setSpriteActive(size_t ipSpriteInx, size_t ipFrameInx, bool bpChangeFrameBuffer, shared_ptr <vector <gametools::sprite::Sprite::FrameDescriptor>> apFrameBuffer) {
            if(ipSpriteInx >= pgSpriteControl[igActiveStateIndex].size())
                throw string("TileInstance::setSpriteActive: Sprite Index out of Range.");
            if(ipFrameInx != -1 && ipFrameInx >= getClass()->getState(igActiveStateIndex)->getSprite(ipSpriteInx)->getFrameCount())
                throw string("TileInstance::setSpriteActive: Frame Index out of Range.");

            SpriteControlDescriptor& rlDescriptor = pgSpriteControl[igActiveStateIndex][ipSpriteInx];

            rlDescriptor.active = true;
            rlDescriptor.finished = false;
            rlDescriptor.stop = false;
            rlDescriptor.reverse = false;
            rlDescriptor.show = true;
            rlDescriptor.frame_inx = ipFrameInx;
            rlDescriptor.tick_accumulator = 0;
            rlDescriptor.pos = rgPosition;
            rlDescriptor.loop_counter = sprite::Sprite::NOLOOP;
            rlDescriptor.loop_index = 0;

            if(bpChangeFrameBuffer || !rlDescriptor.buffer) {
                if(apFrameBuffer) {
                    // In this section times are changed for an animation when a new sprite is activated,
                    // in the case in which the sprite only has a frame then allows to copy that frame apFrameBuffer->size() times and assigns different times according to apFrameBuffer.
                    shared_ptr <const vector <gametools::sprite::Sprite::FrameDescriptor>> alOldFrameBuffer = getClass()->getState(igActiveStateIndex)->getSprite(ipSpriteInx)->getFrames();
                    if((alOldFrameBuffer->size() != 1 && alOldFrameBuffer->size() != apFrameBuffer->size()) || apFrameBuffer->size() == 0)
                        throw string("TileInstance::setSpriteActive: Size of the New Frame Buffer is different from Size of the Original Frame Buffer.");
                    shared_ptr <vector <gametools::sprite::Sprite::FrameDescriptor>> alNewFrameBuffer = make_shared <vector <sprite::Sprite::FrameDescriptor>> (alOldFrameBuffer->size());
                    for(size_t i = 0, j = 0; i < apFrameBuffer->size(); i++, j += alOldFrameBuffer->size() != 1 ? 1 : 0) {
                        if(apFrameBuffer->at(i).division_inx != alOldFrameBuffer->at(j).division_inx)
                            throw string("TileInstance::setSpriteActive: Division Indexes are different between Frame Buffers.");
                        alNewFrameBuffer->at(i) = alOldFrameBuffer->at(j);
                        alNewFrameBuffer->at(i).tick_count = apFrameBuffer->at(i).tick_count;
                        alNewFrameBuffer->at(i).tick_length = apFrameBuffer->at(i).tick_length;
                    }
                    rlDescriptor.buffer = alNewFrameBuffer;
                } else
                    rlDescriptor.buffer = getClass()->getState(igActiveStateIndex)->getSprite(ipSpriteInx)->getFrames();
            }
            rlDescriptor.next_tick_time = chrono::duration_cast <chrono::milliseconds> (chrono::system_clock::now().time_since_epoch()) + rlDescriptor.buffer->at(ipFrameInx).tick_length;
        }

        void TileInstance::setSpriteInactive(size_t ipSpriteInx) {
            if(ipSpriteInx >= pgSpriteControl[igActiveStateIndex].size())
                throw string("TileInstance::setSpriteInactive: Sprite Index out of Range.");
            pgSpriteControl[igActiveStateIndex][ipSpriteInx].active = false;
        }

        bool TileInstance::isSpriteReverse(size_t ipSpriteInx) const {
            if(!pgSpriteControl[igActiveStateIndex][ipSpriteInx].active)
                throw string("TileInstance::isSpriteReverse: Sprite is inactive.");
            if(ipSpriteInx >= pgSpriteControl[igActiveStateIndex].size())
                throw string("TileInstance::isSpriteReverse: Sprite Index out of Range.");
            return pgSpriteControl[igActiveStateIndex][ipSpriteInx].reverse;
        }

        void TileInstance::setSpriteReverse(size_t ipSpriteInx, bool bpReverse) {
            if(!pgSpriteControl[igActiveStateIndex][ipSpriteInx].active)
                throw string("TileInstance::setSpriteReverse: Sprite is inactive.");
            if(ipSpriteInx >= pgSpriteControl[igActiveStateIndex].size())
                throw string("TileInstance::setSpriteReverse: Sprite Index out of Range.");
            pgSpriteControl[igActiveStateIndex][ipSpriteInx].reverse = bpReverse;
        }

        size_t TileInstance::getFrameIndex(size_t ipSpriteInx) const {
            if(!pgSpriteControl[igActiveStateIndex][ipSpriteInx].active)
                throw string("TileInstance::getFrameIndex: Sprite is inactive.");
            if(ipSpriteInx >= pgSpriteControl[igActiveStateIndex].size())
                throw string("TileInstance::getFrameIndex: Sprite Index out of Range.");
            return pgSpriteControl[igActiveStateIndex][ipSpriteInx].frame_inx;
        }

        void TileInstance::setFrameIndex(size_t ipSpriteInx, size_t ipFrameInx) {
            if(!pgSpriteControl[igActiveStateIndex][ipSpriteInx].active)
                throw string("TileInstance::setFrameIndex: Sprite is inactive.");
            if(ipSpriteInx >= pgSpriteControl[igActiveStateIndex].size())
                throw string("TileInstance::setFrameIndex: Sprite Index out of Range.");
            if(ipFrameInx != -1 && ipFrameInx >= getClass()->getState(igActiveStateIndex)->getSprite(ipSpriteInx)->getFrameCount())
                throw string("TileInstance::setFrameIndex: Frame Index out of Range.");
            pgSpriteControl[igActiveStateIndex][ipSpriteInx].frame_inx = ipFrameInx;
        }

        chrono::milliseconds TileInstance::getFrameNextTickTime(size_t ipSpriteInx) const {
            if(!pgSpriteControl[igActiveStateIndex][ipSpriteInx].active)
                throw string("TileInstance::getFrameNextTickTime: Sprite is inactive.");
            if(ipSpriteInx >= pgSpriteControl[igActiveStateIndex].size())
                throw string("TileInstance::getFrameNextTickTime: Sprite Index out of Range.");
            return pgSpriteControl[igActiveStateIndex][ipSpriteInx].next_tick_time;
        }

        void TileInstance::setFrameNextTickTime(size_t ipSpriteInx, chrono::milliseconds tpTime) {
            if(!pgSpriteControl[igActiveStateIndex][ipSpriteInx].active)
                throw string("TileInstance::setFrameNextTickTime: Sprite is inactive.");
            if(ipSpriteInx >= pgSpriteControl[igActiveStateIndex].size())
                throw string("TileInstance::setFrameNextTickTime: Sprite Index out of Range.");
            pgSpriteControl[igActiveStateIndex][ipSpriteInx].next_tick_time = tpTime;
        }

        unsigned long TileInstance::getFrameTickAccumulator(size_t ipSpriteInx) const {
            if(!pgSpriteControl[igActiveStateIndex][ipSpriteInx].active)
                throw string("TileInstance::getFrameTickAccumulator: Sprite is inactive.");
            if(ipSpriteInx >= pgSpriteControl[igActiveStateIndex].size())
                throw string("TileInstance::getFrameTickAccumulator: Sprite Index out of Range.");
            return pgSpriteControl[igActiveStateIndex][ipSpriteInx].tick_accumulator;
        }

        void TileInstance::setFrameTickAccumulator(size_t ipSpriteInx, unsigned long ipValue) {
            if(!pgSpriteControl[igActiveStateIndex][ipSpriteInx].active)
                throw string("TileInstance::setFrameTickAccumulator: Sprite is inactive.");
            if(ipSpriteInx >= pgSpriteControl[igActiveStateIndex].size())
                throw string("TileInstance::setFrameTickAccumulator: Sprite Index out of Range.");
            if(ipValue >= pgSpriteControl[igActiveStateIndex][ipSpriteInx].buffer->at(pgSpriteControl[igActiveStateIndex][ipSpriteInx].frame_inx).tick_count)
                throw string("TileInstance::setFrameTickAccumulator: Value cannot be greater than the Tick Count of the Active Frame.");
            pgSpriteControl[igActiveStateIndex][ipSpriteInx].tick_accumulator = ipValue;
        }

        bool TileInstance::isSpriteFinished(size_t ipSpriteInx) const {
            if(!pgSpriteControl[igActiveStateIndex][ipSpriteInx].active)
                throw string("TileInstance::isSpriteFinished: Sprite is inactive.");
            if(ipSpriteInx >= pgSpriteControl[igActiveStateIndex].size())
                throw string("TileInstance::isSpriteFinished: Sprite Index out of Range.");
            return pgSpriteControl[igActiveStateIndex][ipSpriteInx].finished;
        }

        void TileInstance::setSpriteFinished(size_t ipSpriteInx, bool bpFinished) {
            if(!pgSpriteControl[igActiveStateIndex][ipSpriteInx].active)
                throw string("TileInstance::setSpriteFinished: Sprite is inactive.");
            if(ipSpriteInx >= pgSpriteControl[igActiveStateIndex].size())
                throw string("TileInstance::setSpriteFinished: Sprite Index out of Range.");
            pgSpriteControl[igActiveStateIndex][ipSpriteInx].finished = bpFinished;
        }

        bool TileInstance::isSpriteStoped(size_t ipSpriteInx) const {
            if(!pgSpriteControl[igActiveStateIndex][ipSpriteInx].active)
                throw string("TileInstance::isSpriteStoped: Sprite is inactive.");
            if(ipSpriteInx >= pgSpriteControl[igActiveStateIndex].size())
                throw string("TileInstance::isSpriteStoped: Sprite Index out of Range.");
            return pgSpriteControl[igActiveStateIndex][ipSpriteInx].stop;
        }

        void TileInstance::setSpriteStoped(size_t ipSpriteInx, bool bpStoped) {
            if(!pgSpriteControl[igActiveStateIndex][ipSpriteInx].active)
                throw string("TileInstance::setSpriteStoped: Sprite is inactive.");
            if(ipSpriteInx >= pgSpriteControl[igActiveStateIndex].size())
                throw string("TileInstance::setSpriteStoped: Sprite Index out of Range.");
            pgSpriteControl[igActiveStateIndex][ipSpriteInx].stop = bpStoped;
        }

        AudioCommand TileInstance::getAudioCommand(size_t ipAudioInx) const {
            if(ipAudioInx >= agAudioCommands[igActiveStateIndex].size())
                throw string("TileInstance::getAudioCommand: Audio Index out of Range.");
            return agAudioCommands[igActiveStateIndex][ipAudioInx];
        }

        void TileInstance::setAudioCommand(size_t ipAudioInx, unsigned char ipCommand, bool bpLoop, unsigned int ipLoopLimit, unsigned long ipLoopDelay) {
            if(ipAudioInx >= agAudioCommands[igActiveStateIndex].size())
                throw string("TileInstance::setAudioCommand: Audio Index out of Range.");
            agAudioCommands[igActiveStateIndex][ipAudioInx].command = ipCommand;
            agAudioCommands[igActiveStateIndex][ipAudioInx].loop = bpLoop;
            agAudioCommands[igActiveStateIndex][ipAudioInx].limit = ipLoopLimit;
            agAudioCommands[igActiveStateIndex][ipAudioInx].delay = ipLoopDelay;
        }

        bool TileInstance::executeAudioCommand(size_t ipAudioInx, unsigned char ipCommand, bool bpLoop, unsigned int ipLoopLimit, unsigned long ipLoopDelay) {
            if(!bgIsAudioLoaded)
                throw string("TileInstance::executeSoundCommand: Resources not loaded.");
            if(ipAudioInx >= pgClass->getState(igActiveStateIndex)->getAudioCount())
                throw string("TileInstance::executeSoundCommand: Audio Index out of Bounds.");

            switch(ipCommand) {
                case AudioCommand::NO_COMMAND_SOUND:
                    break;
                case AudioCommand::PLAY_SOUND:
                    return agPlayers[igActiveStateIndex][ipAudioInx]->play(bpLoop, ipLoopLimit, ipLoopDelay);
                case AudioCommand::PAUSE_SOUND:
                    agPlayers[igActiveStateIndex][ipAudioInx]->pause();
                    break;
                case AudioCommand::RESUME_SOUND:
                    agPlayers[igActiveStateIndex][ipAudioInx]->resume();
                    break;
                case AudioCommand::STOP_SOUND:
                    agPlayers[igActiveStateIndex][ipAudioInx]->stop();
                    break;
                case AudioCommand::WAIT_SOUND:
                    agPlayers[igActiveStateIndex][ipAudioInx]->wait();
                    break;
                default:
                    throw string("TileInstance::executeSoundCommand: Command not recognized.");
            }

            return true;
        }

        bool TileInstance::isAudioPlaying(size_t ipAudioInx) {
            if(!bgIsAudioLoaded)
                throw string("TileInstance::isAudioPlaying: Resources not loaded.");
            if(ipAudioInx >= agPlayers[igActiveStateIndex].size())
                throw string("TileInstance::isAudioPlaying: Audio Index out of Bounds.");

            return agPlayers[igActiveStateIndex][ipAudioInx]->isPlaying();
        }

        bool TileInstance::isAudioLooping(size_t ipAudioInx) const {
            if(!bgIsAudioLoaded)
                throw string("TileInstance::isAudioLooping: Resources not loaded.");
            if(ipAudioInx >= agPlayers[igActiveStateIndex].size())
                throw string("TileInstance::isAudioLooping: Audio Index out of Bounds.");

            return agPlayers[igActiveStateIndex][ipAudioInx]->isLooping();
        }

        unsigned int TileInstance::getAudioLoopLimit(size_t ipAudioInx) const {
            if(!bgIsAudioLoaded)
                throw string("TileInstance::getAudioLoopLimit: Resources not loaded.");
            if(ipAudioInx >= agPlayers[igActiveStateIndex].size())
                throw string("TileInstance::getAudioLoopLimit: Audio Index out of Bounds.");

            return agPlayers[igActiveStateIndex][ipAudioInx]->getLoopLimit();
        }

        unsigned int TileInstance::getActualAudioLoop(size_t ipAudioInx) const {
            if(!bgIsAudioLoaded)
                throw string("TileInstance::getActualAudioLoop: Resources not loaded.");
            if(ipAudioInx >= agPlayers[igActiveStateIndex].size())
                throw string("TileInstance::getActualAudioLoop: Audio Index out of Bounds.");

            return agPlayers[igActiveStateIndex][ipAudioInx]->getActualLoop();
        }

        unsigned long TileInstance::getAudioLoopDelay(size_t ipAudioInx) const {
            if(!bgIsAudioLoaded)
                throw string("TileInstance::getAudioLoopDelay: Resources not loaded.");
            if(ipAudioInx >= agPlayers[igActiveStateIndex].size())
                throw string("TileInstance::getAudioLoopDelay: Audio Index out of Bounds.");

            return agPlayers[igActiveStateIndex][ipAudioInx]->getLoopDelay();
        }

        //////////////////////////////////////////////////////////////////////////////////////

        Level::Level(std::shared_ptr <gametools::renderer::ScreenRenderer> opRenderer, function <bool(shared_ptr <Level> opLevel, shared_ptr <const EVENT> opEvent)> ppFunction) {
            if(!opRenderer)
                throw string("Level::Level: Renderer cannot be null.");
            if(!ppFunction)
                throw string("Level::Level: Behaviour Function cannot be null.");

            pgFunction = ppFunction;
            ClassEntry rlTmp;
            rlTmp.tileclass = make_shared <TileClass> ();
            agTiles[0] = rlTmp; // Introduces the 0 class, which is reserved for the comunication with the screen.
            agZBuffer = make_shared <map <long, list <shared_ptr <TileInstance>>>> ();
            bgInitialized = false;
            bgStarted = false;
            bgIsLoaded = false;
            ogRenderer = opRenderer;
            agAudios = make_shared <vector <size_t>> ();
        }

        size_t Level::createIntegerVariable() {
            agIntegerVariables.push_back(0);
            return agIntegerVariables.size() - 1;
        }

        size_t Level::createStringVariable() {
            agStringVariables.push_back("");
            return agStringVariables.size() - 1;
        }

        size_t Level::createFloatVariable() {
            agFloatVariables.push_back(0);
            return agFloatVariables.size() - 1;
        }

        int& Level::getIntegerVariable(size_t ipIntVariableInx) {
            if(ipIntVariableInx >= agIntegerVariables.size())
                throw string("Level::getIntegerVariable: Index out of range.");
            return agIntegerVariables[ipIntVariableInx];
        }

        string& Level::getStringVariable(size_t ipStrVariableInx) {
            if(ipStrVariableInx >= agStringVariables.size())
                throw string("Level::getStringVariable: Index out of range.");
            return agStringVariables[ipStrVariableInx];
        }

        float& Level::getFloatVariable(size_t ipDblVariableInx) {
            if(ipDblVariableInx >= agFloatVariables.size())
                throw string("Level::getFloatVariable: Index out of range.");
            return agFloatVariables[ipDblVariableInx];
        }

        void Level::createLevel() {
            if(bgInitialized)
                throw string("Level::createLevel: Level already initialized.");
            shared_ptr <EVENT> rlCreationEvent = make_shared <EVENT>();
            rlCreationEvent->code = EVENT::LEVEL_CREATION;
            rlCreationEvent->value = 0;
            rlCreationEvent->srcclassid = TileClass::UNINITIALIZED;
            rlCreationEvent->srcinstanceid = TileInstance::UNINITIALIZED;
            rlCreationEvent->dstclassid = TileClass::UNINITIALIZED;
            rlCreationEvent->dstinstanceid = TileClass::UNINITIALIZED;
            gateway(rlCreationEvent, true);
            bgInitialized = true;
        }

        void Level::destroyLevel() {
            if(bgStarted)
                throw string("Level::destroyLevel: Level not ended.");
            if(!bgInitialized)
                throw string("Level::destroyLevel: Level not initialized.");
            shared_ptr <EVENT> rlDestructionEvent = make_shared <EVENT>();
            rlDestructionEvent->code = EVENT::LEVEL_DESTRUCTION;
            rlDestructionEvent->value = 0;
            rlDestructionEvent->srcclassid = TileClass::UNINITIALIZED;
            rlDestructionEvent->srcinstanceid = TileInstance::UNINITIALIZED;
            rlDestructionEvent->dstclassid = TileClass::UNINITIALIZED;
            rlDestructionEvent->dstinstanceid = TileClass::UNINITIALIZED;
            gateway(rlDestructionEvent, true);
            bgInitialized = false;
        }

        void Level::start() {
            if(!bgInitialized)
                throw string("Level::start: Level not initialized.");
            if(bgStarted)
                throw string("Level::start: Level already started.");
            shared_ptr <EVENT> rlCreationEvent = make_shared <EVENT>();
            rlCreationEvent->code = EVENT::LEVEL_START;
            rlCreationEvent->value = 0;
            rlCreationEvent->srcclassid = TileClass::UNINITIALIZED;
            rlCreationEvent->srcinstanceid = TileInstance::UNINITIALIZED;
            rlCreationEvent->dstclassid = TileClass::UNINITIALIZED;
            rlCreationEvent->dstinstanceid = TileClass::UNINITIALIZED;
            gateway(rlCreationEvent, true);
            bgStarted = true;
        }

        void Level::end() {
            if(!bgInitialized)
                throw string("Level::start: Level not initialized.");
            if(!bgStarted)
                throw string("Level::start: Level already ended.");
            shared_ptr <EVENT> rlCreationEvent = make_shared <EVENT>();
            rlCreationEvent->code = EVENT::LEVEL_END;
            rlCreationEvent->value = 0;
            rlCreationEvent->srcclassid = TileClass::UNINITIALIZED;
            rlCreationEvent->srcinstanceid = TileInstance::UNINITIALIZED;
            rlCreationEvent->dstclassid = TileClass::UNINITIALIZED;
            rlCreationEvent->dstinstanceid = TileClass::UNINITIALIZED;
            gateway(rlCreationEvent, true);
            bgStarted = false;
        }

        void Level::setGamepadCount(size_t ipGamepadCount) {
            if(ipGamepadCount == 0)
                throw string("Level::setGamepadState: Gamepad Count cannot be 0.");
            agGamepads = vector <GamepadState> (ipGamepadCount);
        }

        void Level::setGamepadState(size_t ipId, shared_ptr <GamepadState> rpState) {
            if(ipId >= agGamepads.size())
                throw string("Level::setGamepadState: Index out of range.");

            size_t ilDownButton = ((rpState->buttons | agGamepads[ipId].buttons) ^ agGamepads[ipId].buttons) & ~agGamepads[ipId].buttons;
            size_t ilUpButton = ((rpState->buttons | agGamepads[ipId].buttons) ^ rpState->buttons) & ~rpState->buttons;
            size_t ilChangeButton = 
                (rpState->leftTrigger != agGamepads[ipId].leftTrigger ? EVENT::GAMEPAD_LEFT_TRIGGER: 0) |
                (rpState->rightTrigger != agGamepads[ipId].rightTrigger? EVENT::GAMEPAD_RIGHT_TRIGGER: 0) |
                (rpState->thumbLX != agGamepads[ipId].thumbLX || rpState->thumbLY != agGamepads[ipId].thumbLY? EVENT::GAMEPAD_LEFT_THUMB: 0) |
                (rpState->thumbRX != agGamepads[ipId].thumbRX || rpState->thumbRY != agGamepads[ipId].thumbRY? EVENT::GAMEPAD_RIGHT_THUMB: 0) |
                0;

            agGamepads[ipId] = *rpState;

            if(ilDownButton) {
                shared_ptr <EVENT> rlUserInput = make_shared <EVENT>();
                rlUserInput->code = EVENT::USER_INPUT_GAMEPAD_DOWN;
                rlUserInput->value = ilDownButton;
                rlUserInput->srcclassid = TileClass::UNINITIALIZED;
                rlUserInput->srcinstanceid = TileInstance::UNINITIALIZED;
                rlUserInput->dstclassid = TileClass::UNINITIALIZED;
                rlUserInput->dstinstanceid = TileClass::UNINITIALIZED;
                throwEvent(rlUserInput);
            }

            if(ilUpButton) {
                shared_ptr <EVENT> rlUserInput = make_shared <EVENT>();
                rlUserInput->code = EVENT::USER_INPUT_GAMEPAD_UP;
                rlUserInput->value = ilUpButton;
                rlUserInput->srcclassid = TileClass::UNINITIALIZED;
                rlUserInput->srcinstanceid = TileInstance::UNINITIALIZED;
                rlUserInput->dstclassid = TileClass::UNINITIALIZED;
                rlUserInput->dstinstanceid = TileClass::UNINITIALIZED;
                throwEvent(rlUserInput);
            }

            if(ilChangeButton) {
                shared_ptr <EVENT> rlUserInput = make_shared <EVENT>();
                rlUserInput->code = EVENT::USER_INPUT_GAMEPAD_CHANGE;
                rlUserInput->value = ilChangeButton;
                rlUserInput->srcclassid = TileClass::UNINITIALIZED;
                rlUserInput->srcinstanceid = TileInstance::UNINITIALIZED;
                rlUserInput->dstclassid = TileClass::UNINITIALIZED;
                rlUserInput->dstinstanceid = TileClass::UNINITIALIZED;
                throwEvent(rlUserInput);
            }
        }

        shared_ptr <Level::GamepadState> Level::getGamepadState(size_t ipId) const {
            if(ipId >= agGamepads.size())
                throw string("Level::getGamepadState: Index out of range.");
            return make_shared <GamepadState> (agGamepads[ipId]);
        }

        Level::GamepadState::GamepadState() {
            this->buttons = 0;
            this->leftTrigger = 0;
            this->rightTrigger = 0;
            this->thumbLX = 0;
            this->thumbLY = 0;
            this->thumbRX = 0;
            this->thumbRY = 0;
        }

        Level::GamepadState::GamepadState(const XINPUT_STATE& rpState) {
            this->buttons = rpState.Gamepad.wButtons;
            this->leftTrigger = rpState.Gamepad.bLeftTrigger;
            this->rightTrigger = rpState.Gamepad.bRightTrigger;
            this->thumbLX = rpState.Gamepad.sThumbLX;
            this->thumbLY = rpState.Gamepad.sThumbLY;
            this->thumbRX = rpState.Gamepad.sThumbRX;
            this->thumbRY = rpState.Gamepad.sThumbRY;
        }

        Level::GamepadState& Level::GamepadState::operator = (const XINPUT_STATE& rpState) {
            this->buttons = rpState.Gamepad.wButtons;
            this->leftTrigger = rpState.Gamepad.bLeftTrigger;
            this->rightTrigger = rpState.Gamepad.bRightTrigger;
            this->thumbLX = rpState.Gamepad.sThumbLX;
            this->thumbLY = rpState.Gamepad.sThumbLY;
            this->thumbRX = rpState.Gamepad.sThumbRX;
            this->thumbRY = rpState.Gamepad.sThumbRY;

            return *this;
        }

        void Level::keyDown(size_t ipCharacter) {
            shared_ptr <EVENT> rlUserInput = make_shared <EVENT>();
            rlUserInput->code = EVENT::USER_INPUT_KEY_DOWN;
            rlUserInput->value = ipCharacter;
            rlUserInput->srcclassid = TileClass::UNINITIALIZED;
            rlUserInput->srcinstanceid = TileInstance::UNINITIALIZED;
            rlUserInput->dstclassid = TileClass::UNINITIALIZED;
            rlUserInput->dstinstanceid = TileClass::UNINITIALIZED;
            throwEvent(rlUserInput);
        }

        void Level::keyUp(size_t ipCharacter) {
            shared_ptr <EVENT> rlUserInput = make_shared <EVENT>();
            rlUserInput->code = EVENT::USER_INPUT_KEY_UP;
            rlUserInput->value = ipCharacter;
            rlUserInput->srcclassid = TileClass::UNINITIALIZED;
            rlUserInput->srcinstanceid = TileInstance::UNINITIALIZED;
            rlUserInput->dstclassid = TileClass::UNINITIALIZED;
            rlUserInput->dstinstanceid = TileClass::UNINITIALIZED;
            throwEvent(rlUserInput);
        }


        // Dispatch an event to one o several instances, and returns the result of the processing of those events:
        //  If dstclassid != TileClass::UNINITIALIZED && dstinstanceid != TileInstance::UNINITIALIZED then the event is being dispatch to a specific instance, and returns the result produced by that instance.
        //  If dstclassid != TileClass::UNINITIALIZED && dstinstanceid == TileInstance::UNINITIALIZED then the event is dispatch to the class to determine if that event has to be processed by their instances, if It is then dispatches the event to Its instances, and in any case returns the result produced by the class.
        //  If dstclassid == TileClass::UNINITIALIZED the the event is sent to all the instances, always returns true.

        bool Level::gateway(shared_ptr <const EVENT> opEvent, bool bpIsInternalCall) {
            if(!opEvent)
                return false;

            if(!bpIsInternalCall && opEvent->code <= 13) // Checks that a system event is dispatch from the level object.
                return false;

            if(opEvent->srcclassid != TileClass::UNINITIALIZED && !agTiles.count(opEvent->srcclassid))
                throw string("Level::throwEvent: Source Class does not exist.");

            if(opEvent->srcinstanceid != TileInstance::UNINITIALIZED && !agTiles[opEvent->srcclassid].tileinstances.count(opEvent->srcinstanceid))
                throw string("Level::throwEvent: Source Instance does not exist.");

            if(opEvent->dstclassid != TileClass::UNINITIALIZED) {
                if(!agTiles.count(opEvent->dstclassid))
                    throw string("Level::throwEvent: Destination Class does not exist.");

                if(agTiles[opEvent->dstclassid].tileclass->getClassId() == TileClass::UNINITIALIZED)
                    return false;

                if(opEvent->dstclassid == 0) // Clase reservada para la Comunicación con la Pantalla.
                    return pgFunction(const_pointer_cast <Level> (shared_from_this()), opEvent);

                if(opEvent->dstinstanceid != TileInstance::UNINITIALIZED) {
                    if(!agTiles[opEvent->dstclassid].tileinstances.count(opEvent->dstinstanceid))
                        throw string("Level::throwEvent: Destination Instance does not exist.");

                    if(agTiles[opEvent->dstclassid].tileinstances[opEvent->dstinstanceid].instance->getInstanceId() != TileInstance::UNINITIALIZED)
                        return agTiles[opEvent->dstclassid].tileinstances[opEvent->dstinstanceid].instance->catchEvent(opEvent);

                    return false;
                } else {
                    bool blInd = agTiles[opEvent->dstclassid].tileclass->catchEvent(make_shared <TileInstance> (), opEvent);
                    shared_ptr <EVENT> plEvent = make_shared <EVENT> (*opEvent);
                    plEvent->code = EVENT::PROPAGATE_EVENT;
                    plEvent->value = opEvent->code;
                    if(agTiles[opEvent->dstclassid].tileclass->catchEvent(make_shared <TileInstance> (), plEvent)) {
                        for(auto olInstance = agTiles[opEvent->dstclassid].tileinstances.begin(); olInstance != agTiles[opEvent->dstclassid].tileinstances.end(); olInstance++)
                            olInstance->second.instance->catchEvent(opEvent);
                    }
                    return blInd;
                }
            }

            shared_ptr <EVENT> plEvent = make_shared <EVENT> (*opEvent);
            plEvent->code = EVENT::PROPAGATE_EVENT;
            plEvent->value = opEvent->code;
            for(auto olTile = agTiles.begin(); olTile != agTiles.end(); olTile++) {
                if(olTile == agTiles.begin()) {
                    pgFunction(const_pointer_cast <Level> (shared_from_this()), opEvent);
                    continue;
                }
                olTile->second.tileclass->catchEvent(make_shared <TileInstance> (), opEvent);
                if(olTile->second.tileclass->catchEvent(make_shared <TileInstance> (), plEvent))
                    for(auto olInstance = olTile->second.tileinstances.begin(); olInstance != olTile->second.tileinstances.end(); olInstance++)
                        if((opEvent->srcclassid != olTile->second.tileclass->getClassId() || (opEvent->srcclassid == olTile->second.tileclass->getClassId() && opEvent->srcinstanceid != olInstance->first)) && olInstance->second.instance->getInstanceId() != TileInstance::UNINITIALIZED)
                            olInstance->second.instance->catchEvent(opEvent);
            }

            return true;
        }

        bool Level::throwEvent(std::shared_ptr <const EVENT> opEvent) {
            return gateway(opEvent, false);
        }

        size_t Level::registerTileClass(shared_ptr <TileClass> opClass) {
            if(!opClass)
                throw string("Level::registerTileClass: Class cannot be null.");
            if(agTiles.count(opClass->getClassId()))
                return opClass->getClassId();

            ClassEntry rlEntry;
            rlEntry.tileclass = opClass;
            rlEntry.lastinstancekey = 0;
            agTiles[opClass->getClassId()] = rlEntry;

            shared_ptr <EVENT> plEvent = make_shared <EVENT> ();
            plEvent->code = EVENT::CLASS_REGISTRATION;
            plEvent->value = opClass->getAlternativeClassId();
            plEvent->srcclassid = opClass->getClassId();
            plEvent->srcinstanceid = TileInstance::UNINITIALIZED;
            plEvent->dstclassid = TileClass::UNINITIALIZED;
            plEvent->dstinstanceid = TileInstance::UNINITIALIZED;

            gateway(plEvent, true);

            return opClass->getClassId();
        }

        bool Level::unregisterTileClass(size_t ipClassIndex) {
            if(!agTiles.count(ipClassIndex))
                return false;

            shared_ptr <EVENT> plEvent = make_shared <EVENT> ();
            plEvent->code = EVENT::CLASS_UNREGISTRATION;
            plEvent->value = 0;
            plEvent->srcclassid = ipClassIndex;
            plEvent->srcinstanceid = TileInstance::UNINITIALIZED;
            plEvent->dstclassid = TileClass::UNINITIALIZED;
            plEvent->dstinstanceid = TileInstance::UNINITIALIZED;

            gateway(plEvent, true);

            for_each(agTiles[ipClassIndex].tileinstances.begin(), agTiles[ipClassIndex].tileinstances.end(), [this, ipClassIndex] (pair <size_t, InstanceDescriptor> olInstance) {
                this->destroyTileInstance(ipClassIndex, olInstance.first);
            });
            agTiles[ipClassIndex].tileclass = make_shared <TileClass> ();
            agTiles[ipClassIndex].tileinstances.clear();
            agTiles[ipClassIndex].passivecollisionbuffer.clear();
            agTiles.erase(ipClassIndex);

            return true;
        }

        size_t Level::createTileInstance(SI3DPOINT rpPosition, size_t ipClassIndex, shared_ptr <renderer::Renderer> opRenderer, float fpAngle, SF2DPOINT rpScale) {
            if(!agTiles.count(ipClassIndex))
                throw string("Level::createTileInstance: Class does not exist.");

            shared_ptr <TileInstance> olInstance = make_shared <TileInstance> (rpPosition, agTiles[ipClassIndex].tileclass, opRenderer, fpAngle, rpScale);

            olInstance->setInstanceId(agTiles[olInstance->getClass()->getClassId()].lastinstancekey++);
            olInstance->setOwnerLevel(shared_from_this());

            InstanceDescriptor& rlInstanceDescriptor = agTiles[olInstance->getClass()->getClassId()].tileinstances[olInstance->getInstanceId()];
            rlInstanceDescriptor = InstanceDescriptor();
            rlInstanceDescriptor.instance = olInstance;

            shared_ptr <EVENT> plEvent = make_shared <EVENT> ();
            plEvent->code = EVENT::INSTANCE_CREATION;
            plEvent->value = 0;
            plEvent->srcclassid = TileClass::UNINITIALIZED;
            plEvent->srcinstanceid = TileInstance::UNINITIALIZED;
            plEvent->dstclassid = olInstance->getClass()->getClassId();
            plEvent->dstinstanceid = olInstance->getInstanceId();

            if(gateway(plEvent, true)) {
                plEvent->srcclassid = olInstance->getClass()->getClassId();
                plEvent->srcinstanceid = olInstance->getInstanceId();
                plEvent->dstclassid = TileClass::UNINITIALIZED;
                plEvent->dstinstanceid = TileInstance::UNINITIALIZED;

                gateway(plEvent, true);
            }

            plEvent->code = EVENT::CAPABILITY_CHECK;
            plEvent->srcclassid = TileClass::UNINITIALIZED;
            plEvent->srcinstanceid = TileInstance::UNINITIALIZED;
            plEvent->dstclassid = olInstance->getClass()->getClassId();
            plEvent->dstinstanceid = olInstance->getInstanceId();

            plEvent->value = EVENT::IS_DISPLAYABLE;

            if(gateway(plEvent, true)) {
                map <long, list <shared_ptr <TileInstance>>>::iterator it = agZBuffer->emplace(olInstance->getZ(), list <shared_ptr <TileInstance>> ()).first;

                it->second.push_back(olInstance);
                rlInstanceDescriptor.has_zbufferpos = true;
                rlInstanceDescriptor.zpos = olInstance->getZ();
                (rlInstanceDescriptor.zbufferpos = it->second.end())--;
            } else
                rlInstanceDescriptor.has_zbufferpos = false;

            plEvent->value = EVENT::IS_ACTIVE_COLLISION_CAPABLE;

            if(gateway(plEvent, true)) {
                agActiveCollisionBuffer.push_back(olInstance);
                rlInstanceDescriptor.has_acbufferpos = true;
                (rlInstanceDescriptor.acbufferpos = agActiveCollisionBuffer.end())--;
            } else
                rlInstanceDescriptor.has_acbufferpos = false;

            plEvent->value = EVENT::IS_PASSIVE_COLLISION_CAPABLE;

            if(gateway(plEvent, true)) {
                agTiles[olInstance->getClass()->getClassId()].passivecollisionbuffer.push_back(olInstance);
                rlInstanceDescriptor.has_pcbufferpos = true;
                (rlInstanceDescriptor.pcbufferpos = agTiles[olInstance->getClass()->getClassId()].passivecollisionbuffer.end())--;
            } else
                rlInstanceDescriptor.has_pcbufferpos = false;

            return olInstance->getInstanceId();
        }

        bool Level::destroyTileInstance(size_t ipClassIndex, size_t ipInstanceIndex) {
            if(!agTiles.count(ipClassIndex) || !agTiles[ipClassIndex].tileinstances.count(ipInstanceIndex))
                return false;

            shared_ptr <EVENT> plEvent = make_shared <EVENT> ();
            plEvent->code = EVENT::INSTANCE_DESTRUCTION;
            plEvent->value = 0;
            plEvent->srcclassid = TileClass::UNINITIALIZED;
            plEvent->srcinstanceid = TileInstance::UNINITIALIZED;
            plEvent->dstclassid = ipClassIndex;
            plEvent->dstinstanceid = ipInstanceIndex;

            if(gateway(plEvent, true)) {
                plEvent->srcclassid = ipClassIndex;
                plEvent->srcinstanceid = ipInstanceIndex;
                plEvent->dstclassid = TileClass::UNINITIALIZED;
                plEvent->dstinstanceid = TileInstance::UNINITIALIZED;

                gateway(plEvent, true);
            }

            agTiles[ipClassIndex].tileinstances[ipInstanceIndex].instance->setInstanceId(TileInstance::UNINITIALIZED);
            agTiles[ipClassIndex].tileinstances[ipInstanceIndex].instance = make_shared <TileInstance> ();
            if(agTiles[ipClassIndex].tileinstances[ipInstanceIndex].has_zbufferpos) {
                map <long, list <shared_ptr <TileInstance>>>::iterator it = agZBuffer->find(agTiles[ipClassIndex].tileinstances[ipInstanceIndex].zpos);
                it->second.erase(agTiles[ipClassIndex].tileinstances[ipInstanceIndex].zbufferpos);
                agTiles[ipClassIndex].tileinstances[ipInstanceIndex].has_zbufferpos = false;
            }
            if(agTiles[ipClassIndex].tileinstances[ipInstanceIndex].has_acbufferpos) {
                agActiveCollisionBuffer.erase(agTiles[ipClassIndex].tileinstances[ipInstanceIndex].acbufferpos);
                agTiles[ipClassIndex].tileinstances[ipInstanceIndex].has_acbufferpos = false;
            }
            if(agTiles[ipClassIndex].tileinstances[ipInstanceIndex].has_pcbufferpos) {
                agTiles[ipClassIndex].passivecollisionbuffer.erase(agTiles[ipClassIndex].tileinstances[ipInstanceIndex].pcbufferpos);
                agTiles[ipClassIndex].tileinstances[ipInstanceIndex].has_pcbufferpos = false;
            }
            agTiles[ipClassIndex].tileinstances.erase(ipInstanceIndex);

            return true;
        }

        void Level::setAudios(std::shared_ptr <std::vector <size_t>> apAudios) {
            if(!apAudios)
                throw string("Level::setAudios: New Audios Vector is invalid.");
            if(bgIsLoaded && agAudios->size() != 0)
                throw string("Level::setAudios: There are Audios already loaded, unload the Audios before to set new Audios.");
            agAudios = apAudios;
            agAudioCommands = vector <AudioCommand> (agAudios->size(), NULL_AUDIO_COMMAND);
            bgIsLoaded = false;
        }

        size_t Level::getAudioCount() const {
            return agAudios->size();
        }

        bool Level::executeAudioCommand(size_t ipAudioInx, unsigned char ipCommand, bool bpLoop, unsigned int ipLoopLimit, unsigned long ipLoopDelay) {
            if(!bgIsLoaded)
                throw string("Level::executeSoundCommand: Resources not loaded.");
            if(ipAudioInx >= agPlayers.size())
                throw string("Level::executeSoundCommand: Audio Index out of Bounds.");

            switch(ipCommand) {
                case AudioCommand::NO_COMMAND_SOUND:
                    break;
                case AudioCommand::PLAY_SOUND:
                    return agPlayers[ipAudioInx]->play(bpLoop, ipLoopLimit, ipLoopDelay);
                case AudioCommand::PAUSE_SOUND:
                    agPlayers[ipAudioInx]->pause();
                    break;
                case AudioCommand::RESUME_SOUND:
                    agPlayers[ipAudioInx]->resume();
                    break;
                case AudioCommand::STOP_SOUND:
                    agPlayers[ipAudioInx]->stop();
                    break;
                case AudioCommand::WAIT_SOUND:
                    agPlayers[ipAudioInx]->wait();
                    break;
                default:
                    throw string("Level::executeSoundCommand: Command not recognized.");
            }

            return true;
        }

        bool Level::isAudioPlaying(size_t ipAudioInx) {
            if(!bgIsLoaded)
                throw string("Level::isPlaying: Resources not loaded.");
            if(ipAudioInx >= agPlayers.size())
                throw string("Level::isPlaying: Audio Index out of Bounds.");

            return agPlayers[ipAudioInx]->isPlaying();
        }

        bool Level::isAudioLooping(size_t ipAudioInx) const {
            if(!bgIsLoaded)
                throw string("Level::isLooping: Resources not loaded.");
            if(ipAudioInx >= agPlayers.size())
                throw string("Level::isLooping: Audio Index out of Bounds.");

            return agPlayers[ipAudioInx]->isLooping();
        }

        unsigned int Level::getAudioLoopLimit(size_t ipAudioInx) const {
            if(!bgIsLoaded)
                throw string("Level::getLoopLimit: Resources not loaded.");
            if(ipAudioInx >= agPlayers.size())
                throw string("Level::getLoopLimit: Audio Index out of Bounds.");

            return agPlayers[ipAudioInx]->getLoopLimit();
        }

        unsigned int Level::getActualAudioLoop(size_t ipAudioInx) const {
            if(!bgIsLoaded)
                throw string("Level::getActualLoop: Resources not loaded.");
            if(ipAudioInx >= agPlayers.size())
                throw string("Level::getActualLoop: Audio Index out of Bounds.");

            return agPlayers[ipAudioInx]->getActualLoop();
        }

        unsigned long Level::getAudioLoopDelay(size_t ipAudioInx) const {
            if(!bgIsLoaded)
                throw string("Level::getAudioLoopDelay: Resources not loaded.");
            if(ipAudioInx >= agPlayers.size())
                throw string("Level::getAudioLoopDelay: Audio Index out of Bounds.");

            return agPlayers[ipAudioInx]->getLoopDelay();
        }

        AudioCommand Level::getAudioCommand(size_t ipAudioInx) const {
            if(ipAudioInx >= agAudioCommands.size())
                throw string("Level::getAudioCommand: Audio Index out of Range.");
            return agAudioCommands[ipAudioInx];
        }

        void Level::setAudioCommand(size_t ipAudioInx, unsigned char ipCommand, bool bpLoop, unsigned int ipLoopLimit, unsigned long ipLoopDelay) {
            if(ipAudioInx >= agAudioCommands.size())
                throw string("Level::setAudioCommand: Audio Index out of Range.");
            agAudioCommands[ipAudioInx].command = ipCommand;
            agAudioCommands[ipAudioInx].loop = bpLoop;
            agAudioCommands[ipAudioInx].limit = ipLoopLimit;
            agAudioCommands[ipAudioInx].delay = ipLoopDelay;
        }

        void Level::processCollisions() {
            shared_ptr <EVENT> rlEvent = make_shared <EVENT> ();

            // Go over all the tiles that can process active collisions.
            for(auto olActiveInstance = agActiveCollisionBuffer.begin(); olActiveInstance != agActiveCollisionBuffer.end(); olActiveInstance++)
                // Go over all the classes, beginning from 1 to leave off the special class 0 that is used to communicate with the screen.
                for(auto olPassiveClass = next(agTiles.begin(), 1); olPassiveClass != agTiles.end(); olPassiveClass++) {
                    rlEvent->code = EVENT::COLLISION_REQUEST;
                    rlEvent->value = olPassiveClass->second.tileclass->getAlternativeClassId();
                    rlEvent->srcclassid = TileClass::UNINITIALIZED;
                    rlEvent->srcinstanceid = TileInstance::UNINITIALIZED;
                    rlEvent->dstclassid = (*olActiveInstance)->getClass()->getClassId();
                    rlEvent->dstinstanceid = (*olActiveInstance)->getInstanceId();

                    // Checks if the active tile (olInstance) can collide with the passive tiles of the passive class (olClass).
                    if(gateway(rlEvent, true))
                        // Go over all the tiles that process passive collisions in the class classid.
                        for(auto olPassiveInstance = olPassiveClass->second.passivecollisionbuffer.begin(); olPassiveInstance != olPassiveClass->second.passivecollisionbuffer.end(); olPassiveInstance++) {
                            const shared_ptr <const TileState> olActiveState = (*olActiveInstance)->getClass()->getState((*olActiveInstance)->getActiveStateIndex());
                            const shared_ptr <const TileState> olPassiveState = olPassiveClass->second.tileclass->getState((*olPassiveInstance)->getActiveStateIndex());

                            size_t ilActiveTier = 0;
                            bool blActiveInd = true;
                            // Go over all the levels of the borders of the active sprites in olActiveInstance, the 0 level of each sprite is the farthest level while the las level is the closest to the sprite.
                            // The order of the levels according to the borders by defect is sprite::BOXBORDER > sprite::QUADRANTBORDER > sprite::CONTOURBORDER.
                            while(blActiveInd) {
                                blActiveInd = false;
                                
                                for(size_t i = 0; i < olActiveState->getSpriteCount(); i++) {
                                    if((*olActiveInstance)->isSpriteActive(i)) {
                                        size_t ilActiveFrameInx = (*olActiveInstance)->getFrameIndex(i);
                                        SI3DPOINT rlTmpPosDelta;
                                        float flTmpAngleDelta;
                                        SF2DPOINT rlTmpScaleDelta;
                                        if((*olActiveInstance)->getCollisionType() != TileInstance::COLLISION_VECTOR) {
                                            rlTmpPosDelta = (*olActiveInstance)->getPositionDelta();
                                            flTmpAngleDelta = (*olActiveInstance)->getAngleDelta();
                                            rlTmpScaleDelta = (*olActiveInstance)->getScaleDelta();
                                        } else {
                                            rlTmpPosDelta = sprite::DEFAULT_POS_DELTA;
                                            flTmpAngleDelta = sprite::DEFAULT_ANGLE_DELTA;
                                            rlTmpScaleDelta = sprite::DEFAULT_SCALE_DELTA;
                                        }
                                        shared_ptr <CONST_BORDER_REF> alActiveBorders = olActiveState->getFrameBorder(i, ilActiveFrameInx, ilActiveTier, true, (*olActiveInstance)->getPosition(), (*olActiveInstance)->getAngle(), (*olActiveInstance)->getScale(), rlTmpPosDelta, flTmpAngleDelta, rlTmpScaleDelta);

                                        size_t ilPassiveTier = 0;
                                        bool blPassiveInd = true;
                                        // Go over all the levels of the passive borders of olPassiveInstance.
                                        while(blPassiveInd) {
                                            blPassiveInd = false;

                                            for(size_t j = 0; j < olPassiveState->getSpriteCount(); j++) {
                                                if((*olPassiveInstance)->isSpriteActive(j)) {
                                                    size_t ilPassiveFrameInx = (*olPassiveInstance)->getFrameIndex(j);
                                                    shared_ptr <CONST_BORDER_REF> alPassiveBorders = olPassiveState->getFrameBorder(j, ilPassiveFrameInx, ilPassiveTier, false, (*olPassiveInstance)->getPosition(), (*olPassiveInstance)->getAngle(), (*olPassiveInstance)->getScale());

                                                    for(size_t aborder = 0; aborder < alActiveBorders->size(); aborder++)
                                                        for(size_t pborder = 0; pborder < alPassiveBorders->size(); pborder++) {
                                                            if((*olActiveInstance)->getCollisionType() == TileInstance::COLLISION_INSIDE && gametools::utilities::isInsideOf(alPassiveBorders->at(pborder), alActiveBorders->at(aborder))) {
                                                                rlEvent->code = EVENT::PASSIVE_COLLISION;
                                                                rlEvent->value = PACK_COLLISION_DATA(j, ilPassiveFrameInx, ilPassiveTier, pborder);
                                                                rlEvent->srcclassid = (*olActiveInstance)->getClass()->getClassId();
                                                                rlEvent->srcinstanceid = (*olActiveInstance)->getInstanceId();
                                                                rlEvent->dstclassid = (*olPassiveInstance)->getClass()->getClassId();
                                                                rlEvent->dstinstanceid = (*olPassiveInstance)->getInstanceId();
                                                                rlEvent->collision = shared_ptr <BORDER>();
                                                                gateway(rlEvent, true);

                                                                rlEvent->code = EVENT::ACTIVE_COLLISION;
                                                                rlEvent->value = PACK_COLLISION_DATA(i, ilActiveFrameInx, ilActiveTier, aborder);
                                                                rlEvent->srcclassid = (*olPassiveInstance)->getClass()->getClassId();
                                                                rlEvent->srcinstanceid = (*olPassiveInstance)->getInstanceId();
                                                                rlEvent->dstclassid = (*olActiveInstance)->getClass()->getClassId();
                                                                rlEvent->dstinstanceid = (*olActiveInstance)->getInstanceId();
                                                                rlEvent->collision = shared_ptr <BORDER>();
                                                                if(gateway(rlEvent, true)) {
                                                                    blPassiveInd = true;
                                                                    blActiveInd = true;
                                                                }
                                                            } else if((*olActiveInstance)->getCollisionType() == TileInstance::COLLISION_INTERCEPTION || (*olActiveInstance)->getCollisionType() == TileInstance::COLLISION_VECTOR) {
                                                                for(auto olPassivePoint = next(alPassiveBorders->at(pborder)->begin(), 1); olPassivePoint != alPassiveBorders->at(pborder)->end(); olPassivePoint++) {
                                                                    shared_ptr <BORDER> olResult = gametools::utilities::findIntersections(*(olPassivePoint - 1), *olPassivePoint, alActiveBorders->at(aborder));
                                                                    if(olResult->size() != 0) {
                                                                        rlEvent->code = EVENT::PASSIVE_COLLISION;
                                                                        rlEvent->value = PACK_COLLISION_DATA(j, ilPassiveFrameInx, ilPassiveTier, pborder);
                                                                        rlEvent->srcclassid = (*olActiveInstance)->getClass()->getClassId();
                                                                        rlEvent->srcinstanceid = (*olActiveInstance)->getInstanceId();
                                                                        rlEvent->dstclassid = (*olPassiveInstance)->getClass()->getClassId();
                                                                        rlEvent->dstinstanceid = (*olPassiveInstance)->getInstanceId();
                                                                        rlEvent->collision = olResult;
                                                                        gateway(rlEvent, true);

                                                                        rlEvent->code = EVENT::ACTIVE_COLLISION;
                                                                        rlEvent->value = PACK_COLLISION_DATA(i, ilActiveFrameInx, ilActiveTier, aborder);
                                                                        rlEvent->srcclassid = (*olPassiveInstance)->getClass()->getClassId();
                                                                        rlEvent->srcinstanceid = (*olPassiveInstance)->getInstanceId();
                                                                        rlEvent->dstclassid = (*olActiveInstance)->getClass()->getClassId();
                                                                        rlEvent->dstinstanceid = (*olActiveInstance)->getInstanceId();
                                                                        rlEvent->collision = olResult;
                                                                        if(gateway(rlEvent, true)) {
                                                                            blPassiveInd = true;
                                                                            blActiveInd = true;
                                                                        }
                                                                    }
                                                                }
                                                                shared_ptr <BORDER> olResult = gametools::utilities::findIntersections(alPassiveBorders->at(pborder)->back(), alPassiveBorders->at(pborder)->front(), alActiveBorders->at(aborder));
                                                                if(olResult->size() != 0) {
                                                                    rlEvent->code = EVENT::PASSIVE_COLLISION;
                                                                    rlEvent->value = PACK_COLLISION_DATA(j, ilPassiveFrameInx, ilPassiveTier, pborder);
                                                                    rlEvent->srcclassid = (*olActiveInstance)->getClass()->getClassId();
                                                                    rlEvent->srcinstanceid = (*olActiveInstance)->getInstanceId();
                                                                    rlEvent->dstclassid = (*olPassiveInstance)->getClass()->getClassId();
                                                                    rlEvent->dstinstanceid = (*olPassiveInstance)->getInstanceId();
                                                                    rlEvent->collision = olResult;
                                                                    gateway(rlEvent, true);

                                                                    rlEvent->code = EVENT::ACTIVE_COLLISION;
                                                                    rlEvent->value = PACK_COLLISION_DATA(i, ilActiveFrameInx, ilActiveTier, aborder);
                                                                    rlEvent->srcclassid = (*olPassiveInstance)->getClass()->getClassId();
                                                                    rlEvent->srcinstanceid = (*olPassiveInstance)->getInstanceId();
                                                                    rlEvent->dstclassid = (*olActiveInstance)->getClass()->getClassId();
                                                                    rlEvent->dstinstanceid = (*olActiveInstance)->getInstanceId();
                                                                    rlEvent->collision = olResult;
                                                                    if(gateway(rlEvent, true)) {
                                                                        blPassiveInd = true;
                                                                        blActiveInd = true;
                                                                    }
                                                                }
                                                            }

                                                        }
                                                }
                                            }
                                            ilPassiveTier++;
                                        }
                                    }
                                }
                                ilActiveTier++;
                            }
                        }
                }
        }

        bool Level::isInitialized() const {
            return bgInitialized;
        }

        bool Level::isLoaded() const {
            return bgIsLoaded;
        }

        void Level::loadResources(bool bpAsynchronousLoad) {
            for(auto entry = agTiles.begin(); entry != agTiles.end(); entry++)
                entry->second.tileclass->loadResources(bpAsynchronousLoad);

            if(agAudios->size() != 0)
                ResourceManager::getInstance()->loadAudios(agAudios);

            if(!bpAsynchronousLoad)
                waitResourcesLoad();
        }

        void Level::unloadResources(bool bpUnloadAudios) {
            for(auto entry = agTiles.begin(); entry != agTiles.end(); entry++) {
                entry->second.tileclass->unloadResources(bpUnloadAudios);
                if(bpUnloadAudios)
                    for(auto instance = entry->second.tileinstances.begin(); instance != entry->second.tileinstances.end(); instance++)
                        instance->second.instance->unloadAudios();
            }

            if(bpUnloadAudios) {
                agPlayers.clear();
                ResourceManager::getInstance()->unloadAudios(agAudios);
                bgIsLoaded = false;
            }
        }

        bool Level::isLoadingAudio() {
            for(auto entry = agTiles.begin(); entry != agTiles.end(); entry++)
                if(entry->second.tileclass->isLoadingAudio())
                    return true;

            if(ResourceManager::getInstance()->isLoadingAudios())
                return true;

            waitResourcesLoad();

            return false;
        }

        void Level::waitResourcesLoad() {
            if(bgIsLoaded)
                return;

            for(auto entry = agTiles.begin(); entry != agTiles.end(); entry++) {
                entry->second.tileclass->waitResourcesLoad();
                for(auto instance = entry->second.tileinstances.begin(); instance != entry->second.tileinstances.end(); instance++)
                    instance->second.instance->loadAudios();
            }

            shared_ptr <ResourceManager> olManager = ResourceManager::getInstance();
            olManager->waitAudiosLoad();
            for(size_t i = 0; i < agAudios->size(); i++)
                agPlayers.push_back(make_shared <AsynchronousPlayer>(olManager->getAudio(agAudios->at(i))));

            bgIsLoaded = true;
        }

        void Level::playLevel(bool bpDrawBorder, bool bpActive, unsigned char ipPixelSize, RGBA rlBorderColor) {
            if(!bgInitialized)
                throw string("Level::renderLevel: Level not initialized.");
            if(!bgStarted)
                throw string("Level::renderLevel: Level not started.");
            if(!bgIsLoaded)
                throw string("Level::renderLevel: Resources not loaded.");

            // Lanza el Evento DRAW a todas las Instancias, y prepara el Buffer Z.
            shared_ptr <map <long, list <shared_ptr <TileInstance>>>> alTmpZBuffer = make_shared <map <long, list <shared_ptr <TileInstance>>>> ();
            for(auto& tiles: *agZBuffer) {
                for(auto& tile: tiles.second) {
                    shared_ptr <EVENT> rlEvent = make_shared <EVENT> ();
                    rlEvent->code = EVENT::DRAW;
                    rlEvent->value = 0;
                    rlEvent->srcclassid = TileClass::UNINITIALIZED;
                    rlEvent->srcinstanceid = TileInstance::UNINITIALIZED;
                    rlEvent->dstclassid = tile->getClass()->getClassId();
                    rlEvent->dstinstanceid = tile->getInstanceId();
                    gateway(rlEvent, true);

                    map <long, list <shared_ptr <TileInstance>>>::iterator it = alTmpZBuffer->emplace(tile->getZ(), list <shared_ptr <TileInstance>> ()).first;
                    it->second.push_back(tile);
                    agTiles[tile->getClass()->getClassId()].tileinstances[tile->getInstanceId()].zpos = tile->getZ();
                    (agTiles[tile->getClass()->getClassId()].tileinstances[tile->getInstanceId()].zbufferpos = it->second.end())--;
                }
            }
            // Process the collisions and informs to the affected instances for them to recalculate the positions and change their states, do not modify the Z buffer.
            processCollisions();
            // Prints the corresponding sprites of each instance.
            agZBuffer = alTmpZBuffer;
            ogRenderer->prepareScreen(getScreen().width, getScreen().height);
            for(auto& tiles : *agZBuffer)
                for(auto& tile : tiles.second)
                    tile->playInstance(bpDrawBorder, bpActive, ipPixelSize, rlBorderColor);

            for(size_t i = 0; i < agAudios->size(); i++) {
                AudioCommand rlCommand = getAudioCommand(i);
                if(rlCommand.command != AudioCommand::NO_COMMAND_SOUND) {
                    executeAudioCommand(i, rlCommand.command, rlCommand.loop, rlCommand.limit, rlCommand.delay);
                    setAudioCommand(i, AudioCommand::NO_COMMAND_SOUND);
                }
            }

            ogRenderer->drawScreen();
        }

        void Level::setScreenRenderer(std::shared_ptr <gametools::renderer::ScreenRenderer> opRenderer) {
            if(!opRenderer)
                throw string("Level::setRenderer: Renderer cannot be null.");

            ogRenderer = opRenderer;
        }

        std::shared_ptr <gametools::renderer::ScreenRenderer> Level::getPostRenderer() {
            return ogRenderer;
        }

        void Level::setScreen(SCREEN& rpScreen, bool blIsSubScreen) {
            if(!blIsSubScreen && (GetSystemMetrics(SM_CXSCREEN) != rpScreen.width || GetSystemMetrics(SM_CYSCREEN) != rpScreen.height)) {
                DEVMODE rlMode;

                rlMode.dmPosition.x = rpScreen.originX;
                rlMode.dmPosition.y = rpScreen.originY;
                rlMode.dmPelsWidth = rpScreen.width;
                rlMode.dmPelsHeight = rpScreen.height;
                rlMode.dmFields = DM_PELSHEIGHT | DM_PELSWIDTH | DM_POSITION;
                switch(ChangeDisplaySettings(&rlMode, CDS_FULLSCREEN)) {
                    case DISP_CHANGE_BADFLAGS:
                    case DISP_CHANGE_BADMODE:
                        throw string("Level::setScreen: An error has occured while trying to change screen resolution.");
                        break;
                    case DISP_CHANGE_FAILED:
                        throw string("Level::setScreen: The required resolution is not supported..");
                        break;
                }
            }
            rgScreen = rpScreen;
        }

        const SCREEN& Level::getScreen() const {
            return rgScreen;
        }

        shared_ptr <RGBAImage> loadImage(string slBitmapPath, unsigned char igMode, bool bpPlaceholder = false) {
            string slExtension = slBitmapPath.substr(slBitmapPath.size() - 4);
            for_each(slExtension.begin() + 1, slExtension.end(), [](char& c) {
                c = tolower(c);
            });

            bool blIsPNG = slExtension.compare(".png") == 0;
            shared_ptr <RGBAImage> olTmpImage = blIsPNG ? ImageTools::readPNGFromFile(slBitmapPath, bpPlaceholder) : ImageTools::readBMPFromFile(slBitmapPath, bpPlaceholder);
            if(!bpPlaceholder && igMode == ResourceManager::GDI_ALPHA_MODE)
                for(unsigned long y = 0; y < olTmpImage->getHeight(); y++)
                    for(unsigned long x = 0; x < olTmpImage->getWidth(); x++) {
                        RGBA rlPixel = olTmpImage->getPixel(x, y);
                        rlPixel.r = (rlPixel.r * rlPixel.a) / 255;
                        rlPixel.g = (rlPixel.g * rlPixel.a) / 255;
                        rlPixel.b = (rlPixel.b * rlPixel.a) / 255;
                        olTmpImage->setRealPixel(x, y, rlPixel);
                    }
            return olTmpImage;
        }

        shared_ptr <Audio> loadAudio(string slAudioPath) {
            string slExtension = slAudioPath.substr(slAudioPath.size() - 4);
            for_each(slExtension.begin() + 1, slExtension.end(), [](char& c) {
                c = tolower(c);
            });

            shared_ptr <Audio> olTmpAudio;
            if(slExtension.compare(".wav") != 0)
                olTmpAudio = AudioTools::readMIDIFromFile(slAudioPath);
            else
                olTmpAudio = AudioTools::readWAVFromFile(slAudioPath);

            return olTmpAudio;
        }

        ResourceManager::ResourceManager(unsigned char ipMode, HWND hpWindow, std::function <void()> ppLibraryInitializationFunction) {
            if(ipMode != GDI_ALPHA_MODE && ipMode != GDI_MASK_MODE && ipMode != OGL_ALPHA_MODE && ipMode != OGL_MASK_MODE)
                throw string("ResourceManager::ResourceManager: Mode not defined.");

            igMode = ipMode;

            if(ipMode == GDI_ALPHA_MODE || ipMode == GDI_MASK_MODE)
                setDeviceContext(hpWindow);
            else if(ipMode == OGL_ALPHA_MODE || ipMode == OGL_MASK_MODE) {
                if(!setRenderContext(hpWindow))
                    throw string("ResourceManager::ResourceManager: Cannot initialize Open GL Context.");

                ppLibraryInitializationFunction();
                glEnable(GL_BLEND);
                // Set "clearing" or background color
                glClearColor(0.5f, 0.5f, 0.5f, 1.0f);

                // Program for AlphaRenderer and MaskRenderer.
                string slVertexShader =
                    "#version 330 core\n"
                    "layout (location = 0) in vec3 aPos;\n"
                    "layout (location = 1) in vec2 aTexCoord;\n"
                    "uniform mat4 aPerspective;\n"
                    "uniform mat4 aTransformation;\n"
                    "uniform float aPointSize;\n"
                    "out vec2 bTexCoord;\n"
                    "void main()\n"
                    "{\n"
                    "   gl_Position = aPerspective * aTransformation * vec4(aPos, 1.0);\n"
                    "   gl_PointSize = aPointSize;\n"
                    "   bTexCoord = aTexCoord;\n"
                    "}\0";

                string slFragmentShader =
                    "#version 330 core\n"
                    "uniform sampler2D bTexture;\n"
                    "uniform float aPointSize;\n"
                    "uniform vec3 aPointColor;\n"
                    "in vec2 bTexCoord;\n"
                    "void main()\n"
                    "{\n"
                    "   gl_FragColor = aPointSize != 0.0 ? vec4(aPointColor, 1.0): texture(bTexture, bTexCoord);\n"
                    "}\0";

                string slGeometryShader = "";

                compileShaderProgram(slVertexShader, slGeometryShader, slFragmentShader);

                // Program for MaskByColorRenderer, that uses the new fragment shadder but reuses the shader of vertexes of AlphaRenderer and MaskRenderer.
                slFragmentShader =
                    "#version 330 core\n"
                    "uniform sampler2D bTexture;\n"
                    "uniform float aPointSize;\n"
                    "uniform vec3 aPointColor;\n"
                    "uniform vec3 aMaskColor;\n"
                    "in vec2 bTexCoord;\n"
                    "void main()\n"
                    "{\n"
                    "   if(aPointSize != 0.0) {\n"
                    "       gl_FragColor = vec4(aPointColor, 1.0);\n"
                    "   } else {\n"
                    "       gl_FragColor = texture(bTexture, bTexCoord);\n"
                    "       if(aMaskColor.rgb == gl_FragColor.rgb) {\n"
                    "           gl_FragColor = vec4(1.0, 1.0, 1.0, 0.0);\n"
                    "       }\n"
                    "   }\n"
                    "}\0";

                compileShaderProgram(slVertexShader, slGeometryShader, slFragmentShader);
            }

            // This introduces as the first bitmap (index 0 = CONTROL_BITMAP_INX) an image of 1x1 that serves as the base for the control sprites.
            BitmapDescriptor rlTmp;

            rlTmp.bitmap = make_shared <RGBAImage> (1, 1, vector <unsigned char> ({0, 0, 0, 0}));
            rlTmp.isMemoryReloadable = false;
            rlTmp.isCacheReloadable = false;
            rlTmp.path = "AUTO";
            rlTmp.handler = NULL;
            rlTmp.mask_inx = CONTROL_BITMAP_INX; // This assigns a mask to Itself to prevent from other mask to be assigned.
            rlTmp.background = BLACK;
            agBitmaps.push_back(rlTmp);
        }

        bool ResourceManager::setPixelFormat() {
            PIXELFORMATDESCRIPTOR rlPixelFormatDescriptor;
            int ilPixelFormat;

            rlPixelFormatDescriptor.nSize = sizeof(PIXELFORMATDESCRIPTOR);
            rlPixelFormatDescriptor.nVersion = 1;
            rlPixelFormatDescriptor.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
            rlPixelFormatDescriptor.dwLayerMask = PFD_MAIN_PLANE;
            rlPixelFormatDescriptor.iPixelType = PFD_TYPE_RGBA;
            rlPixelFormatDescriptor.cColorBits = 24;
            rlPixelFormatDescriptor.cDepthBits = 16;
            rlPixelFormatDescriptor.cAccumBits = 0;
            rlPixelFormatDescriptor.cStencilBits = 0;

            ilPixelFormat = ChoosePixelFormat(hgScreen, &rlPixelFormatDescriptor);

            if((ilPixelFormat = ChoosePixelFormat(hgScreen, &rlPixelFormatDescriptor)) == 0) {
                MessageBox(NULL, L"ChoosePixelFormat failed", L"Error", MB_OK);
                return false;
            }

            if(SetPixelFormat(hgScreen, ilPixelFormat, &rlPixelFormatDescriptor) == FALSE) {
                MessageBox(NULL, L"SetPixelFormat failed", L"Error", MB_OK);
                return false;
            }

            return true;
        }

        void ResourceManager::loadBitmapsAsynchronous(std::shared_ptr <const std::vector <size_t>> apBitmapInx) {
            if(!ogLoadBitmapsMutex.try_lock())
                throw string("ResourceManager::loadBitmapsAsynchronous: Cannot lock Mutex to initiate Loading Process.");
            wglMakeCurrent(hgScreen, hgSecondaryContext);

            for(size_t i = 0; i < apBitmapInx->size(); i++) { // Start in 1 to leave out the bitmap of the control sprites that are at position 0 (CONTROL_BITMAP_INX).
                if(apBitmapInx->at(i) >= agBitmaps.size())
                    throw string("ResourceManager::loadBitmapsAsynchronous: Bitmap Index out of Bounds.");

                if(agBitmaps[apBitmapInx->at(i)].bitmap->isPlaceholder() && agBitmaps[apBitmapInx->at(i)].path.compare("AUTO") != 0) {
                    agBitmaps[apBitmapInx->at(i)].bitmap = loadImage(agBitmaps[apBitmapInx->at(i)].path, igMode);
                    if((igMode == GDI_MASK_MODE || igMode == OGL_MASK_MODE)) {
                        if(agBitmaps[apBitmapInx->at(i)].mask_inx != -1) {
                            if(agBitmaps[agBitmaps[apBitmapInx->at(i)].mask_inx].bitmap->isPlaceholder()) {
                                if(agBitmaps[agBitmaps[apBitmapInx->at(i)].mask_inx].path.compare("AUTO") != 0)
                                    agBitmaps[agBitmaps[apBitmapInx->at(i)].mask_inx].bitmap = loadImage(agBitmaps[agBitmaps[apBitmapInx->at(i)].mask_inx].path, igMode);
                                else {
                                    agBitmaps[agBitmaps[apBitmapInx->at(i)].mask_inx].bitmap = gametools::utilities::getMaskedBitmap(agBitmaps[apBitmapInx->at(i)].bitmap, true, igMode != GDI_MASK_MODE, agBitmaps[agBitmaps[apBitmapInx->at(i)].mask_inx].background);
                                    agBitmaps[apBitmapInx->at(i)].bitmap = gametools::utilities::getMaskedBitmap(agBitmaps[apBitmapInx->at(i)].bitmap, false, false, agBitmaps[agBitmaps[apBitmapInx->at(i)].mask_inx].background);
                                }
                            }
                        } else
                            makeMask(apBitmapInx->at(i), agBitmaps[apBitmapInx->at(i)].background);
                    }
                }

                if(agBitmaps[apBitmapInx->at(i)].handler == NULL) {
                    if(igMode == GDI_ALPHA_MODE || igMode == GDI_MASK_MODE)
                        agBitmaps[apBitmapInx->at(i)].handler = (size_t) gametools::utilities::writeToGDIBuffer(hgScreen, agBitmaps[apBitmapInx->at(i)].bitmap);
                    else if(igMode == OGL_ALPHA_MODE || igMode == OGL_MASK_MODE)
                        agBitmaps[apBitmapInx->at(i)].handler = gametools::utilities::writeToOGLBuffer(agBitmaps[apBitmapInx->at(i)].bitmap);
                    if((igMode == GDI_MASK_MODE || igMode == OGL_MASK_MODE) && agBitmaps[apBitmapInx->at(i)].mask_inx != -1 && agBitmaps[agBitmaps[apBitmapInx->at(i)].mask_inx].handler == NULL) {
                        if(igMode == GDI_MASK_MODE)
                            agBitmaps[agBitmaps[apBitmapInx->at(i)].mask_inx].handler = (size_t) gametools::utilities::writeToGDIBuffer(hgScreen, agBitmaps[agBitmaps[apBitmapInx->at(i)].mask_inx].bitmap);
                        else if(igMode == OGL_MASK_MODE)
                            agBitmaps[agBitmaps[apBitmapInx->at(i)].mask_inx].handler = gametools::utilities::writeToOGLBuffer(agBitmaps[agBitmaps[apBitmapInx->at(i)].mask_inx].bitmap);
                    }
                }
            }

            wglMakeCurrent(NULL, NULL);
            ogLoadBitmapsMutex.unlock();
        }

        void ResourceManager::loadAudiosAsynchronous(std::shared_ptr <const std::vector <size_t>> apAudioInx) {
            if(!ogLoadAudiosMutex.try_lock())
                throw string("ResourceManager::loadAudiosAsynchronous: Cannot lock Mutex to initiate Loading Process.");

            for(size_t i = 0; i < apAudioInx->size(); i++) {
                if(apAudioInx->at(i) >= agAudios.size())
                    throw string("ResourceManager::loadAudios: Index out of Bounds.");
                if(!agAudios[apAudioInx->at(i)].audio) {
                    agAudios[apAudioInx->at(i)].handle = -1;
                    agAudios[apAudioInx->at(i)].audio = loadAudio(agAudios[apAudioInx->at(i)].path);
                }
            }

            ogLoadAudiosMutex.unlock();
        }

        const std::function <void ()> ResourceManager::NULL_INITIALIZATION_FUNCTION = []() {
            return;
        };

        ResourceManager::~ResourceManager() {
            destroy();
        }

        shared_ptr <ResourceManager> ResourceManager::getInstance(unsigned char ipMode, HWND hpWindow, std::function <void()> ppLibraryInitializationFunction) {
            static std::shared_ptr <ResourceManager> ogInstance(new ResourceManager(ipMode, hpWindow, ppLibraryInitializationFunction)); // This kind of variable is only initialized the first time the function is executed, and It is done in a threadsafe manner.
            return ogInstance;
        }

        void ResourceManager::clear() {
            if(igMode == GDI_ALPHA_MODE || igMode == GDI_MASK_MODE) {
                for(size_t i = 0; i < agBitmaps.size(); i++)
                    if(agBitmaps[i].handler != NULL)
                        DeleteObject((HBITMAP) agBitmaps[i].handler);
            } else if(igMode == OGL_ALPHA_MODE || igMode == OGL_MASK_MODE) {
                for(size_t i = 0; i < agBitmaps.size(); i++)
                    if(agBitmaps[i].handler != NULL)
                        glDeleteTextures(1, (unsigned int*) &agBitmaps[i].handler);
            }
            agBitmaps.clear();
            agAudios.clear();
        }

        void ResourceManager::destroy() {
            clear();
            if(igMode == GDI_ALPHA_MODE || igMode == GDI_MASK_MODE) {
                if(hgScreen) {
                    ReleaseDC(hgWindow, hgScreen);
                    hgScreen = NULL;
                }
            } else if(igMode == OGL_ALPHA_MODE || igMode == OGL_MASK_MODE) {
                for(size_t i = 0; i < agPrograms.size(); i++)
                    glDeleteProgram(agPrograms[i]);
                if(hgSecondaryContext) {
                    wglDeleteContext(hgSecondaryContext);
                    hgSecondaryContext = NULL;
                }
                if(hgPrimaryContext) {
                    wglDeleteContext(hgPrimaryContext);
                    hgPrimaryContext = NULL;
                }
                if(hgScreen) {
                    ReleaseDC(hgWindow, hgScreen);
                    hgScreen = NULL;
                }
            }
        }

        unsigned char ResourceManager::getMode() {
            return igMode;
        }
        
        void ResourceManager::setDeviceContext(HWND hpWindow) {
            if(igMode != GDI_ALPHA_MODE && igMode != GDI_MASK_MODE)
                throw string("ResourceManager::addDeviceContext: Not in GDI_ALPHA_MODE or GDI_MASK_MODE.");

            if(hpWindow == NULL)
                throw string("ResourceManager::addDeviceContext: GDI Window Handle is null.");

            hgWindow = hpWindow;
            hgScreen = GetDC(hgWindow);
            hgPrimaryContext = NULL;
            hgSecondaryContext = NULL;
        }

        bool ResourceManager::setRenderContext(HWND hpWindow) {
            if(igMode != OGL_ALPHA_MODE && igMode != OGL_MASK_MODE)
                throw string("ResourceManager::setRenderingContext: Not in OGL_ALPHA_MODE or OGL_MASK_MODE.");
            
            if(hpWindow == NULL)
                throw string("ResourceManager::setRenderingContext: GDI Window Handler is null.");

            hgWindow = hpWindow;

            hgScreen = GetDC(hgWindow);
            if(!setPixelFormat())
                return false;

            hgPrimaryContext = wglCreateContext(hgScreen);
            hgSecondaryContext = wglCreateContext(hgScreen);
            if(wglShareLists(hgPrimaryContext, hgSecondaryContext) == FALSE) {
                DWORD ilErrorCode = GetLastError();
                LPVOID lpMsgBuf;
                FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS, NULL, ilErrorCode, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPTSTR) &lpMsgBuf, 0, NULL);
                MessageBox(NULL, (LPCTSTR) lpMsgBuf, L"Error", MB_OK | MB_ICONINFORMATION);
                LocalFree(lpMsgBuf);
                // Destroy the GL context and just use 1 GL context.
                wglDeleteContext(hgSecondaryContext);
            }
            wglMakeCurrent(hgScreen, hgPrimaryContext);

            return true;
        }

        HDC ResourceManager::getDeviceContext() {
            return hgScreen;
        }

        HWND ResourceManager::getWindowHandle() {
            return hgWindow;
        }

        size_t ResourceManager::addBitmap(
            string spBitmapFile,
            const tools::image::RGBA& rpBackgroundColor,
            bool bpMemoryReloadable,
            bool bpCacheReloadable
        ) {
            BitmapDescriptor rlTmp;

            if(spBitmapFile.compare("") == 0)
                throw string("ResourceManager::addImage: Image File Name is empty.");
            string slExtension = spBitmapFile.substr(spBitmapFile.size() - 4);
            for_each(slExtension.begin() + 1, slExtension.end(), [](char& c) {
                c = tolower(c);
            });

            bool blIsPNG = slExtension.compare(".png") == 0;
            if(!blIsPNG && slExtension.compare(".bmp") != 0)
                throw string("ResourceManager::addImage: Image Format is not PNG or BMP.");
            rlTmp.bitmap = loadImage(spBitmapFile, igMode, true);
            rlTmp.isMemoryReloadable = bpMemoryReloadable;
            rlTmp.isCacheReloadable = bpMemoryReloadable || bpCacheReloadable;
            rlTmp.path = spBitmapFile;
            rlTmp.handler = NULL;
            rlTmp.mask_inx = -1;
            rlTmp.background = rpBackgroundColor;
            agBitmaps.push_back(rlTmp);

            return agBitmaps.size() - 1;
        }

        size_t ResourceManager::addAudio(
            std::string spAudioFile,
            bool bpReloadable // Must unload and load again when the image is not being used?
        ) {
            AudioDescriptor rlTmp;

            if(spAudioFile.compare("") == 0)
                throw string("ResourceManager::addAudio: Audio File Name is empty.");
            string slExtension = spAudioFile.substr(spAudioFile.size() - 4);
            for_each(slExtension.begin() + 1, slExtension.end(), [](char& c) {
                c = tolower(c);
            });
            if(slExtension.compare(".wav") != 0 && slExtension.compare(".mid") != 0)
                throw string("ResourceManager::addAudio: Audio Format is not WAV or MIDI.");
            rlTmp.audio = shared_ptr <tools::audio::Audio> ();
            rlTmp.isReloadable = bpReloadable;
            rlTmp.path = spAudioFile;
            rlTmp.handle = bpReloadable? NULL: -1;
            agAudios.push_back(rlTmp);

            return agAudios.size() - 1;
        }

        size_t ResourceManager::makeMask(size_t ipBitmapInx, const tools::image::RGBA& ipBackgroundColor) {
            if(igMode != GDI_MASK_MODE && igMode != OGL_MASK_MODE)
                throw string("ResourceManager::makeMask: Mode must be GDI_MASK_MODE or OGL_MASK_MODE.");
            if(ipBitmapInx >= agBitmaps.size())
                throw string("ResourceManager::makeMask: Index out of Range.");
            if(agBitmaps[ipBitmapInx].mask_inx != -1)
                throw string("ResourceManager::makeMask: Mask already assigned.");
            if(agBitmaps[ipBitmapInx].bitmap->isPlaceholder())
                throw string("ResourceManager::makeMask: Bitmap is a Placeholder.");
            BitmapDescriptor rlTmp;

            rlTmp.bitmap = gametools::utilities::getMaskedBitmap(agBitmaps[ipBitmapInx].bitmap, true, igMode != GDI_MASK_MODE, ipBackgroundColor);
            rlTmp.isMemoryReloadable = agBitmaps[ipBitmapInx].isMemoryReloadable;
            rlTmp.isCacheReloadable = agBitmaps[ipBitmapInx].isCacheReloadable;
            rlTmp.path = "AUTO";
            rlTmp.handler = NULL;
            rlTmp.mask_inx = -1;
            rlTmp.background = ipBackgroundColor;
            agBitmaps[ipBitmapInx].bitmap = gametools::utilities::getMaskedBitmap(agBitmaps[ipBitmapInx].bitmap, false, false, ipBackgroundColor);
            agBitmaps[ipBitmapInx].mask_inx = agBitmaps.size();
            //////////////////////////////////////////
            // BUG VS 2017
            // The insertion of the mask into the bitmaps list is put here because a bug that occurs when the capacity of the vector has to be incremented, which causes that the parameter ipBackgroundColor points to a different location of memory because Its value is passed by reference.
            agBitmaps.push_back(rlTmp);
            //////////////////////////////////////////

            return agBitmaps[ipBitmapInx].mask_inx;
        }

        void ResourceManager::associateMask(size_t ipBitmapInx, size_t ipMaskInx) {
            if(ipBitmapInx >= agBitmaps.size())
                throw string("ResourceManager::associateMask: Bitmap Index out of Range.");
            if(ipMaskInx >= agBitmaps.size())
                throw string("ResourceManager::associateMask: Mask Index out of Range.");
            if(agBitmaps[ipMaskInx].mask_inx != -1)
                throw string("ResourceManager::associateMask: Mask cannot have another Mask.");
            shared_ptr <RGBAImage> olTmpImage = agBitmaps[ipBitmapInx].bitmap;
            shared_ptr <RGBAImage> olTmpMask = agBitmaps[ipMaskInx].bitmap;
            if(olTmpMask->getHeight() != olTmpImage->getHeight() || olTmpMask->getWidth() != olTmpImage->getWidth())
                throw string("ResourceManager::associateMask: Mask Image must have the Same Size as the Sprite Image.");
            agBitmaps[ipMaskInx].isMemoryReloadable = agBitmaps[ipBitmapInx].isMemoryReloadable;
            agBitmaps[ipMaskInx].isCacheReloadable = agBitmaps[ipBitmapInx].isCacheReloadable;
            agBitmaps[ipBitmapInx].mask_inx = ipMaskInx;
        }

        size_t ResourceManager::getMaskIndex(size_t ipBitmapInx) {
            if(ipBitmapInx >= agBitmaps.size())
                throw string("ResourceManager::getMaskIndex: Image do not exist.");
            if(agBitmaps[ipBitmapInx].mask_inx == -1)
                throw string("ResourceManager::getMaskIndex: Mask not assigned.");
            return agBitmaps[ipBitmapInx].mask_inx;
        }

        void ResourceManager::loadBitmaps(shared_ptr <const vector <size_t>> apBitmapInx) {
            if(!apBitmapInx)
                throw string("ResourceManager::loadBitmaps: Bitmap Index Vector cannot be null.");

            if(agBitmaps.size() < apBitmapInx->size())
                throw string("ResourceManager::loadBitmaps: Number of Indexes is major than the Number of Bitmaps.");

            if((igMode == GDI_ALPHA_MODE || igMode == GDI_MASK_MODE) && hgScreen == NULL)
                throw string("ResourceManager::loadBitmaps: Device Context is null.");

            if(!ogLoadBitmapsMutex.try_lock())
                throw string("ResourceManager::loadBitmaps: Cannot lock Mutex to initiate Loading Process.");
            ogLoadBitmapsMutex.unlock();

            if(ogLoadBitmapsThread.joinable())
                ogLoadBitmapsThread.join();
            ogLoadBitmapsThread = thread(&ResourceManager::loadBitmapsAsynchronous, this, apBitmapInx);
        }

        void ResourceManager::unloadBitmaps(shared_ptr <const vector <size_t>> apBitmapInx) {
            if(!apBitmapInx)
                throw string("ResourceManager::unloadBitmaps: Bitmap Index Vector cannot be null.");
            if(agBitmaps.size() < apBitmapInx->size())
                throw string("ResourceManager::unloadBitmap: Number of Indexes is major than the Number of Bitmaps.");

            for(size_t i = 1; i < apBitmapInx->size(); i++) { // Start in 1 to leave out the bitmap of the control sprites that are at position 0 (CONTROL_BITMAP_INX).
                if(apBitmapInx->at(i) >= agBitmaps.size())
                    throw string("ResourceManager::unloadBitmap: Bitmap Index out of Bounds.");

                if(agBitmaps[apBitmapInx->at(i)].isCacheReloadable && agBitmaps[apBitmapInx->at(i)].handler != NULL) {
                    if(igMode == GDI_ALPHA_MODE || igMode == GDI_MASK_MODE)
                        DeleteObject((HBITMAP) agBitmaps[apBitmapInx->at(i)].handler);
                    else if(igMode == OGL_ALPHA_MODE || igMode == OGL_MASK_MODE)
                        glDeleteTextures(1, (const unsigned int*) &agBitmaps[apBitmapInx->at(i)].handler);
                    agBitmaps[apBitmapInx->at(i)].handler = NULL;
                    if((igMode == GDI_MASK_MODE || igMode == OGL_MASK_MODE) && agBitmaps[apBitmapInx->at(i)].mask_inx != -1 && agBitmaps[agBitmaps[apBitmapInx->at(i)].mask_inx].handler != NULL) {
                        if(igMode == GDI_MASK_MODE)
                            DeleteObject((HBITMAP) agBitmaps[agBitmaps[apBitmapInx->at(i)].mask_inx].handler);
                        else if(igMode == OGL_MASK_MODE)
                            glDeleteTextures(1, (const unsigned int*) &agBitmaps[agBitmaps[apBitmapInx->at(i)].mask_inx].handler);
                        agBitmaps[agBitmaps[apBitmapInx->at(i)].mask_inx].handler = NULL;
                    }
                }

                if(agBitmaps[apBitmapInx->at(i)].isMemoryReloadable && !agBitmaps[apBitmapInx->at(i)].bitmap->isPlaceholder()) {
                    agBitmaps[apBitmapInx->at(i)].bitmap = make_shared <RGBAImage> (agBitmaps[apBitmapInx->at(i)].bitmap->getWidth(), agBitmaps[apBitmapInx->at(i)].bitmap->getHeight(), true);
                    if((igMode == GDI_MASK_MODE || igMode == OGL_MASK_MODE) && agBitmaps[apBitmapInx->at(i)].mask_inx != -1 && agBitmaps[agBitmaps[apBitmapInx->at(i)].mask_inx].bitmap)
                        agBitmaps[agBitmaps[apBitmapInx->at(i)].mask_inx].bitmap = make_shared <RGBAImage> (agBitmaps[agBitmaps[apBitmapInx->at(i)].mask_inx].bitmap->getWidth(), agBitmaps[agBitmaps[apBitmapInx->at(i)].mask_inx].bitmap->getHeight(), true);
                }
            }
        }

        bool ResourceManager::isLoadingBitmaps() {
            if(ogLoadBitmapsMutex.try_lock()) {
                ogLoadBitmapsMutex.unlock();
                return false;
            }
            return true;
        }

        void ResourceManager::waitBitmapsLoad() {
            if(ogLoadBitmapsThread.joinable())
                ogLoadBitmapsThread.join();
        }

        void ResourceManager::loadAudios(std::shared_ptr <const std::vector <size_t>> apAudioInx) {
            if(!apAudioInx)
                throw string("ResourceManager::loadAudios: Audio Index Vector cannot be null.");

            if(agAudios.size() < apAudioInx->size())
                throw string("ResourceManager::loadAudios: Number of Indexes is major than the Number of Audios.");

            if(!ogLoadAudiosMutex.try_lock())
                throw string("ResourceManager::loadAudios: Cannot lock Mutex to initiate Loading Process.");
            ogLoadAudiosMutex.unlock();

            if(ogLoadAudiosThread.joinable())
                ogLoadAudiosThread.join();
            ogLoadAudiosThread = thread(&ResourceManager::loadAudiosAsynchronous, this, apAudioInx);
        }

        void ResourceManager::unloadAudios(std::shared_ptr <const std::vector <size_t>> apAudioInx) {
            if(!apAudioInx)
                throw string("ResourceManager::unloadAudios: Audio Index Vector cannot be null.");
            if(agAudios.size() < apAudioInx->size())
                throw string("ResourceManager::unloadAudios: Number of Indexes is major than the Number of Audios.");

            for(size_t i = 0; i < apAudioInx->size(); i++) {
                if(apAudioInx->at(i) >= agAudios.size())
                    throw string("ResourceManager::unloadAudios: Audio Index out of Bounds.");
                if(agAudios[apAudioInx->at(i)].isReloadable && agAudios[apAudioInx->at(i)].audio) {
                    agAudios[apAudioInx->at(i)].audio = shared_ptr <Audio> (nullptr);
                    agAudios[apAudioInx->at(i)].handle = NULL;
                }
            }
        }

        bool ResourceManager::isLoadingAudios() {
            if(ogLoadAudiosMutex.try_lock()) {
                ogLoadAudiosMutex.unlock();
                return false;
            }
            return true;
        }

        void ResourceManager::waitAudiosLoad() {
            if(ogLoadAudiosThread.joinable())
                ogLoadAudiosThread.join();
        }

        size_t ResourceManager::compileShaderProgram(std::string spVertexShader, std::string spGeometryShader, std::string spFragmentShader) {
            if(spVertexShader.compare("") == 0)
                throw string("ResourceManager::compileShaderProgram: Vertex Shader cannot be empty.");
            if(spFragmentShader.compare("") == 0)
                throw string("ResourceManager::compileShaderProgram: Fragment Shader cannot be empty.");

            // build and compile our shader program
            // ------------------------------------
            // vertex shader.
            int hlVertexShader = glCreateShader(GL_VERTEX_SHADER);
            const char* plTmp = spVertexShader.c_str();
            glShaderSource(hlVertexShader, 1, &plTmp, NULL);
            glCompileShader(hlVertexShader);
            // check for shader compile errors.
            int ilSuccess;
            char alInfoLog[512];
            glGetShaderiv(hlVertexShader, GL_COMPILE_STATUS, &ilSuccess);
            if(!ilSuccess) {
                glGetShaderInfoLog(hlVertexShader, 512, NULL, alInfoLog);
                throw string("ResourceManager::compileShaderProgram: Compilation Error of Vertex Shader: ") + string(alInfoLog);
            }
            // fragment shader
            int hlFragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
            plTmp = spFragmentShader.c_str();
            glShaderSource(hlFragmentShader, 1, &plTmp, NULL);
            glCompileShader(hlFragmentShader);
            // check for shader compile errors.
            glGetShaderiv(hlFragmentShader, GL_COMPILE_STATUS, &ilSuccess);
            if(!ilSuccess) {
                glGetShaderInfoLog(hlFragmentShader, 512, NULL, alInfoLog);
                throw string("ResourceManager::compileShaderProgram: Compilation Error of Fragment Shader: ") + string(alInfoLog);
            }
            // geometry shader
            bool blIsGeometryShader = spGeometryShader.compare("") != 0;
            int hlGeometryShader = 0;
            if(blIsGeometryShader) {
                hlGeometryShader = glCreateShader(GL_GEOMETRY_SHADER);
                plTmp = spGeometryShader.c_str();
                glShaderSource(hlGeometryShader, 1, &plTmp, NULL);
                glCompileShader(hlGeometryShader);
                // check for shader compile errors.
                glGetShaderiv(hlGeometryShader, GL_COMPILE_STATUS, &ilSuccess);
                if(!ilSuccess) {
                    glGetShaderInfoLog(hlGeometryShader, 512, NULL, alInfoLog);
                    throw string("ResourceManager::compileShaderProgram: Compilation Error of Fragment Shader: ") + string(alInfoLog);
                }
            }
            // link shaders.
            unsigned int hlProgram = glCreateProgram();
            glAttachShader(hlProgram, hlVertexShader);
            glAttachShader(hlProgram, hlFragmentShader);
            if(blIsGeometryShader)
                glAttachShader(hlProgram, hlGeometryShader);
            glLinkProgram(hlProgram);
            // check for linking errors.
            glGetProgramiv(hlProgram, GL_LINK_STATUS, &ilSuccess);
            if(!ilSuccess) {
                glGetProgramInfoLog(hlProgram, 512, NULL, alInfoLog);
                throw string("ResourceManager::compileShaderProgram: Linking Error of Shader Program: ") + string(alInfoLog);
            }
            glDeleteShader(hlVertexShader);
            glDeleteShader(hlFragmentShader);
            agPrograms.push_back(hlProgram);

            return agPrograms.size() - 1;
        }

        unsigned int ResourceManager::getShaderProgramHandle(size_t ipProgramInx) {
            if(igMode != OGL_ALPHA_MODE && igMode != OGL_MASK_MODE)
                throw string("ResourceManager::getShaderProgramHandle: The Mode must be OGL_ALPHA_MODE or OGL_MASK_MODE.");
            if(ipProgramInx >= agPrograms.size())
                throw string("ResourceManager::getShaderProgramHandle: Index out of Range.");
            return agPrograms[ipProgramInx];
        }

        shared_ptr <RGBAImage> ResourceManager::getBitmap(size_t ipInx) {
            if(agBitmaps.size() <= ipInx)
                throw string("ResourceManager::getBitmap: Index is major than the Number of Bitmaps.");
            return agBitmaps[ipInx].bitmap;
        }

        size_t ResourceManager::getBitmapHandle(size_t ipInx) {
            if(agBitmaps.size() <= ipInx)
                throw string("ResourceManager::getBitmap: Index is major than the Number of Bitmaps.");
            return agBitmaps[ipInx].handler;
        }

        shared_ptr <Audio> ResourceManager::getAudio(size_t ipInx) {
            if(agAudios.size() <= ipInx)
                throw string("ResourceManager::getAudio: Index is major than the Number of Audios.");
            return agAudios[ipInx].audio;
        }

    } // END CONTEINER

    //////////////////////////////////////////////////////////////////////////////////////
    // CONVERSION TOOLS

    namespace utilities {

        BITMAPV4HEADER initInfo(unsigned long ipHeight, unsigned long ipWidth, size_t lpSize) {
            BITMAPV4HEADER rlHeader;
    
            rlHeader.bV4Size = sizeof(BITMAPV4HEADER);
            rlHeader.bV4Width = ipWidth;
            rlHeader.bV4Height = ipHeight;
            rlHeader.bV4Planes = 1;
            rlHeader.bV4BitCount = 32;
            rlHeader.bV4V4Compression = BI_RGB;
            rlHeader.bV4SizeImage = (unsigned long) lpSize;
            rlHeader.bV4XPelsPerMeter = 0;
            rlHeader.bV4YPelsPerMeter = 0;
            rlHeader.bV4ClrUsed = 0;
            rlHeader.bV4ClrImportant = 0;
            rlHeader.bV4RedMask = 0;
            ((unsigned char*) &rlHeader.bV4RedMask)[2] = 255; // {0, 0, 255, 0}
            rlHeader.bV4GreenMask = 0;
            ((unsigned char*) &rlHeader.bV4GreenMask)[1] = 255; // {0, 255, 0, 0}
            rlHeader.bV4BlueMask = 0;
            ((unsigned char*) &rlHeader.bV4BlueMask)[0] = 255; // {255, 0, 0, 0}
            rlHeader.bV4AlphaMask = 0;
            ((unsigned char*) &rlHeader.bV4AlphaMask)[3] = 255; // {0, 0, 0, 255}
            // {'R', 'G', 'B', 's'}
            ((char*) &rlHeader.bV4CSType)[3] = 'R';
            ((char*) &rlHeader.bV4CSType)[2] = 'G';
            ((char*) &rlHeader.bV4CSType)[1] = 'B';
            ((char*) &rlHeader.bV4CSType)[0] = 's';
            rlHeader.bV4Endpoints = {{0, 0, 0}, {0, 0, 0}, {0, 0, 0}};
            rlHeader.bV4GammaRed = 0;
            rlHeader.bV4GammaGreen = 0;
            rlHeader.bV4GammaBlue = 0;

            return rlHeader;
        }

        HBITMAP writeToGDIBuffer(HDC hpDeviceContext, shared_ptr <const RGBAImage> opImage) {
            BITMAPV4HEADER rlHeader = initInfo(opImage->getHeight(), opImage->getWidth(), opImage->getRawData().size());
            BITMAPINFO rlInfo;

            rlInfo.bmiHeader = *((BITMAPINFOHEADER*) &rlHeader);
    
            return CreateDIBitmap(
                hpDeviceContext,
                (BITMAPINFOHEADER*) &rlHeader,
                CBM_INIT,
                &opImage->getRawData()[0],
                &rlInfo,
                DIB_RGB_COLORS
            );
        }

        unsigned int writeToOGLBuffer(std::shared_ptr <const tools::image::RGBAImage> opImage, bool bpGenerateMipmap) {
            unsigned int hlTexture;
            glGenTextures(1, &hlTexture);
            glBindTexture(GL_TEXTURE_2D, hlTexture);
            // set the texture wrapping parameters.
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);	// set texture wrapping to GL_REPEAT (default wrapping method)
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
            // set texture filtering parameters.
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            // load image, create texture and generate mipmaps.
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, opImage->getWidth(), opImage->getHeight(), 0, GL_BGRA, GL_UNSIGNED_BYTE, &opImage->getRawData()[0]);
            if(bpGenerateMipmap)
                glGenerateMipmap(GL_TEXTURE_2D);

            return hlTexture;
        }

        shared_ptr <BORDER> getImageTightContourBorder(
            shared_ptr <const BaseImage> opImage,
            function <bool(const RGBA&)> ppFunc, // Function that checks if the current point is on the border.
            unsigned char ipSegmentLength, // Distance between the elements of the border, but the distance between the first and last point of the border may be shorter.
            unsigned int ipBorderSize, // Maximum Number of points on the border, if the number of points is enough then It is possible that the distance between the first and last point on the border is greater than ipSegmentLength.
            unsigned int ipBackTrace // Number of points that are considered for proximity search.
        ) {
            if(!opImage)
                throw string("GameTools::getImageTightContourBorder: Image cannot be null.");
            if(!ppFunc)
                throw string("GameTools::getImageTightContourBorder: Evaluation Function cannot be null.");
            if(opImage->isPlaceholder())
                throw string("GameTools::getImageTightContourBorder: Image is a Placeholder.");

            // Adjusts the size of the segment to 9% of the shortest side of the image, if the result is 0 then sets Is on OUTLINE_SEGMENT_LENGTH points, and if It is greater than 255 sets It on 255 points.
            if(ipSegmentLength == 0) {
                unsigned long ilTmp = ((opImage->getHeight() < opImage->getWidth() ? opImage->getHeight() : opImage->getWidth()) / 100) * 9;
                if(ilTmp < OUTLINE_SEGMENT_LENGTH)
                    ilTmp = OUTLINE_SEGMENT_LENGTH;
                else if(ilTmp > 255)
                    ilTmp = 255;
                ipSegmentLength = (unsigned char) ilTmp;
            }

            BORDER alSearchPath; // Circular search route around the last point on the border that's followed to find the next point.
            long ilNextX = -ipSegmentLength; // Starts the route on the left limit of the X axis.
            long ilNextY = 0;
            long ilNewY = ilNextY;

            // Finds the second quadrant of the circular search route and puts It on alSearchPath.
            while(alSearchPath.size() == 0 || ilNextX != 0 || ilNextY != ipSegmentLength) {
                alSearchPath.push_back({ilNextX, ilNextY});

                // ilNewY do not allow that points that belong to the route stay outside of It when they share the same X.
                if(ilNewY == ilNextY) {
                    ilNextX++;
                    ilNewY = (long) sqrt((ipSegmentLength * ipSegmentLength) - (ilNextX * ilNextX));
                    // Finds if there are several points on the same X, checking that the difference between the new point and the previous point on Y is not greater than 1.
                    if(abs(ilNextY - ilNewY) > 1) {
                        if(ilNextY > ilNewY)
                            ilNextY--;
                        else
                            ilNextY++;
                    } else
                        ilNextY = ilNewY;
                } else {
                    if(ilNextY > ilNewY)
                        ilNextY--;
                    else
                        ilNextY++;
                }
            }
            alSearchPath.push_back({0, ipSegmentLength});

            // Complements the first quadrant of the route reflecting the second quadrant, and complements the third and fouth quadrants reflecting the first and second quadrants.
            for(size_t cp = 0; cp < 2; cp++)
                for(size_t q = 0, size = alSearchPath.size() - 1, last = alSearchPath.size() - 2; q < size; q++)
                    if(cp == 0)
                        alSearchPath.push_back({-alSearchPath[last - q].x, alSearchPath[last - q].y});
                    else
                        alSearchPath.push_back({alSearchPath[last - q].x, -alSearchPath[last - q].y});
            alSearchPath.erase(alSearchPath.end() - 1);

            shared_ptr <BORDER> alBorder = make_shared <BORDER> ();

            unsigned long ilInitialSearchLine = opImage->getHeight() / 2;
            long ilCenterX;
            long ilCenterY;

            // Picks up the starting point.
            unsigned long j = ilInitialSearchLine;
            bool blInd = true;
            while(blInd) {
                for(unsigned long i = 0; i < opImage->getWidth(); i++)
                    if(ppFunc(opImage->getPixel(i, j))) {
                        ilCenterX = (long) i;
                        ilCenterY = (long) j;
                        alBorder->push_back({ilCenterX, ilCenterY});
                        blInd = false;
                        break;
                    }
                j = j < opImage->getHeight() - 1 ? j + 1 : 0;
                if(j == ilInitialSearchLine)
                    throw string("ImageTools::getImageCanonicalBorders: There is not a Valid Image to outline.");
            }

            // Controls the maximum number of cycles that can be done in order to avoid a infinite loop.
            unsigned long ilCiclesLimit = (unsigned long) ((float) ipBorderSize * atan(1) * (float) 4 * (float) (ipSegmentLength << 1));
            unsigned long ilCicleCounter = 0;

            // Finds the next index in alCicle.
            size_t ilNextInx = 0;
            size_t ilPathStep = 0;
            size_t ilQuarter = alSearchPath.size() / 4;

            while(ilCicleCounter++ < ilCiclesLimit && alBorder->size() < ipBorderSize && ilPathStep < alSearchPath.size() - 1 && (alBorder->size() <= 2 || (alBorder->size() > 2 && sqrt(((alBorder->at(0).y - ilCenterY) * (alBorder->at(0).y - ilCenterY)) + ((alBorder->at(0).x - ilCenterX) * (alBorder->at(0).x - ilCenterX))) > ipSegmentLength))) {
                ilPathStep++; // Variable to control when the end of the route is reached.
                ilNextInx = ilNextInx > 0? ilNextInx - 1: alSearchPath.size() - 1; // Moves through the route counterclockwise, It must be considered that the Y coordinate is inverted.
                ilNextX = alSearchPath[ilNextInx].x + ilCenterX;
                ilNextY = alSearchPath[ilNextInx].y + ilCenterY;
                if(ilNextX >= 0 && ilNextY >= 0 && ilNextX < (long) opImage->getWidth() && ilNextY < (long) opImage->getHeight() && ppFunc(opImage->getPixel(ilNextX, ilNextY))) {
                    // Checks if there are points on the border that are inside the area defined by the route centered at the current point without taking in consideration the previous point, if there are then does not include the current point on the border and continues the search.
                    bool blJump = false;
                    // Starts the search from the oldest point determined by ilBackTrace up to the penultimate point.
                    for(long tmp = ipBackTrace; alBorder->size() > ipBackTrace + 1 && tmp; tmp--) {
                        // 2 is subtracted because the center that produced the current point cannot be considered.
                        long ilDX = alBorder->at(alBorder->size() - tmp - 2).x - ilNextX;
                        long ilDY = alBorder->at(alBorder->size() - tmp - 2).y - ilNextY;
                        if(sqrt(ilDX * ilDX + ilDY * ilDY) <= ipSegmentLength) {
                            blJump = true;
                            break;
                        }
                    }
                    if(blJump)
                        continue;
                    // The new center is the found point
                    ilCenterX = ilNextX;
                    ilCenterY = ilNextY;
                    // and the new center is added to the border.
                    alBorder->push_back({ilCenterX, ilCenterY});
                    // Picks up the point at the intersection of the routes defined by the old center and the new center,
                    // whichs corresponds to a free point that has being confirmed between both circumferences, such that It is located counterclockwise with respect to the old center,
                    // and clockwise with respect to the new center, and It serves as the start of the search of the route defined by the new center.
                    ilNextInx = ilQuarter + ilNextInx >= alSearchPath.size() ? ilQuarter - (alSearchPath.size() - ilNextInx): ilNextInx + ilQuarter;
                    ilPathStep = 0;
                }
            }

            return alBorder;
        }

        shared_ptr <BORDER> getImageLooseContourBorder(
            shared_ptr <const BaseImage> opImage,
            std::function <bool(const RGBA&)> ppFunc // Function that validates if a point is on the border.
        ) {
            if(!opImage)
                throw string("GameTools::getImageLooseContourBorder: Image cannot be null.");
            if(!ppFunc)
                throw string("GameTools::getImageLooseContourBorder: Evaluation Function cannot be null.");
            if(opImage->isPlaceholder())
                throw string("GameTools::getImageLooseContourBorder: Image is a Placeholder.");

            shared_ptr <BORDER> alBorder = make_shared <BORDER>();

            // Searches on the image along a line at 90 degrees with respect to the X axis from left to right and from bottom to top until It finds the first point that satisfies ppFunc to form the first point of this segment.
            bool blInd = false;
            for(unsigned long i = 0; i < opImage->getWidth(); i++) {
                for(unsigned long j = 0; j < opImage->getHeight(); j++)
                    if(ppFunc(opImage->getPixel(i, j))) {
                        alBorder->push_back({(long) i, (long) j});
                        blInd = true;
                        break;
                    }
                if(blInd) {
                    // On the line that was picked up searches from top to bottom until It finds the first point that satisfies ppFunc to forms the second point of this segment.
                    for(unsigned long j = opImage->getHeight() - 1; j != -1; j--)
                        if(ppFunc(opImage->getPixel(i, j))) {
                            SI3DPOINT rlTmpPoint = {(long) i, (long) j};
                            if(alBorder->back().y != rlTmpPoint.y) // If this point is different to the previous one, in other word if their Y coordinates are different, then adds the point.
                                alBorder->push_back(rlTmpPoint);
                            break;
                        }
                    break;
                }
            }

            if(alBorder->size() != 0) { // If It found a point on the last step then there may be more points to find.
                // Searches on the Image along a line at 45 degrees with respect to the X axis and 90 degrees with respect to the line that intersects the upper left corner, going on from left to right and from bottom to top until finds the first point that satisfies ppFunc that will serve as the first point of this segment.
                blInd = false;
                for(unsigned long io = 0, jo = opImage->getHeight() - 1; io < opImage->getWidth(); io++) {
                    bool blLoop = true;
                    while(jo >= 0 && blLoop) {
                        for(unsigned long i = io, j = jo; i < opImage->getWidth() && j < opImage->getHeight(); i++, j++)
                            if(ppFunc(opImage->getPixel(i, j))) {
                                SI3DPOINT rlTmpPoint = {(long) i, (long) j};
                                if(alBorder->back().x != rlTmpPoint.x || alBorder->back().y != rlTmpPoint.y) // If this point is different from the previous one, in other words if their coordinates X and/or Y are different, then It adds the point to the border.
                                    alBorder->push_back(rlTmpPoint);
                                blInd = true;
                                break;
                            }
                        if(blInd) {
                            // On the selected line searches from top to bottom until It finds the first point that satisfies ppFunc that will serve as the second point of this segment.
                            unsigned long ilXEnd = 0;
                            unsigned long ilYEnd = 0;
                            bool blAlt = opImage->getHeight() < opImage->getWidth();
                            if(blAlt) {
                                ilXEnd = opImage->getHeight() - jo + io;
                                if(ilXEnd >= opImage->getWidth()) {
                                    ilXEnd = opImage->getWidth() - 1;
                                    ilYEnd = opImage->getHeight() - io;
                                } else
                                    ilYEnd = opImage->getHeight() - 1;
                            } else {
                                ilYEnd = opImage->getWidth() - io + jo;
                                if(ilYEnd >= opImage->getHeight()) {
                                    ilXEnd = opImage->getHeight() - jo;
                                    ilYEnd = opImage->getWidth() - 1;
                                } else
                                    ilXEnd = opImage->getWidth() - 1;
                            }
                            for(unsigned long i = ilXEnd, j = ilYEnd; i != -1 && j != -1; i--, j--)
                                if(ppFunc(opImage->getPixel(i, j))) {
                                    SI3DPOINT rlTmpPoint = {(long) i, (long) j};
                                    if(alBorder->back().x != rlTmpPoint.x || alBorder->back().y != rlTmpPoint.y) // If this point is different from the previous one, in other words if their coordinates X and/or Y are different, then It adds the point to the border.
                                        alBorder->push_back(rlTmpPoint);
                                    break;
                                }
                            break;
                        } else if(jo >= 0)
                            jo--;
                        else
                            blLoop = false;
                    }
                    if(blInd)
                        break;
                }

                // Searches on the image on the line at 90 degrees with respect to the Y axis from bottom to top and from left to right until It finds the first point that satisfies ppFunc that will serve as the first point of this segment.
                blInd = false;
                for(unsigned long j = opImage->getHeight() - 1; j != -1 ; j--) {
                    for(unsigned long i = 0; i < opImage->getWidth(); i++)
                        if(ppFunc(opImage->getPixel(i, j))) {
                            SI3DPOINT rlTmpPoint = {(long) i, (long) j};
                            if(alBorder->back().x != rlTmpPoint.x || alBorder->back().y != rlTmpPoint.y) // If this point is different from the previous one, in other words if their coordinates X and/or Y are different, then It adds the point to the border.
                                alBorder->push_back(rlTmpPoint);
                            blInd = true;
                            break;
                        }
                    if(blInd) {
                        // On the selected line searches from right to left until It finds the first point that satisfies ppFunc that will serve as the second point of this segment.
                        for(unsigned long i = opImage->getWidth() - 1; i >= 0; i--)
                            if(ppFunc(opImage->getPixel(i, j))) {
                                SI3DPOINT rlTmpPoint = {(long) i, (long) j};
                                if(alBorder->back().x != rlTmpPoint.x) // If this point is different from the previous one, in other words if their coordinates X and/or Y are different, then It adds the point to the border.
                                    alBorder->push_back(rlTmpPoint);
                                break;
                            }
                        break;
                    }
                }

                // Searches on the image on the line at 45 degrees with respecto to the X axis and at 90 degrees with respect to the line that intersects the upper right corner, going on from left to right and from top to bottom unitl It finds the first point that satisfies ppFunc that will serve as the firs point of this segment.
                blInd = false;
                for(unsigned long io = opImage->getWidth() - 1, jo = opImage->getHeight() - 1; jo != -1; io--) {
                    bool blLoop = true;
                    while(io >= 0 && blLoop) {
                        for(unsigned long i = io, j = jo; i < opImage->getWidth() && j != -1; i++, j--)
                            if(ppFunc(opImage->getPixel(i, j))) {
                                SI3DPOINT rlTmpPoint = {(long) i, (long) j};
                                if(alBorder->back().x != rlTmpPoint.x || alBorder->back().y != rlTmpPoint.y) // If this point is different from the previous one, in other words if their coordinates X and/or Y are different, then It adds the point to the border.
                                    alBorder->push_back(rlTmpPoint);
                                blInd = true;
                                break;
                            }
                        if(blInd) {
                            // On the selected line searches from bottom to top until It finds the first point that satisfies ppFunc that will serve as the second point of this segment.
                            unsigned long ilXEnd = 0;
                            unsigned long ilYEnd = 0;
                            bool blAlt = opImage->getHeight() < opImage->getWidth();
                            if(blAlt) {
                                ilXEnd = opImage->getHeight() - jo + io;
                                if(ilXEnd >= opImage->getWidth()) {
                                    ilXEnd = opImage->getWidth() - 1;
                                    ilYEnd = opImage->getHeight() - io;
                                } else
                                    ilYEnd = opImage->getHeight() - 1;
                            } else {
                                ilYEnd = opImage->getWidth() - io + jo;
                                if(ilYEnd >= opImage->getHeight()) {
                                    ilXEnd = opImage->getHeight() - jo;
                                    ilYEnd = opImage->getWidth() - 1;
                                } else
                                    ilXEnd = opImage->getWidth() - 1;
                            }
                            for(unsigned long i = ilXEnd, j = ilYEnd; i != -1 && j < opImage->getHeight(); i--, j++)
                                if(ppFunc(opImage->getPixel(i, j))) {
                                    SI3DPOINT rlTmpPoint = {(long) i, (long) j};
                                    if(alBorder->back().x != rlTmpPoint.x || alBorder->back().y != rlTmpPoint.y) // If this point is different from the previous one, in other words if their coordinates X and/or Y are different, then It adds the point to the border.
                                        alBorder->push_back(rlTmpPoint);
                                    break;
                                }
                            break;
                        } else if(io >= 0)
                            io--;
                        else
                            blLoop = false;
                    }
                    if(blInd)
                        break;
                }

                // Searches on the image on the line at 90 degrees with respect to the X axis from right to left and from top to bottom until It finds the first point that satisfies ppFunc that will serve as the first point of this segment.
                blInd = false;
                for(unsigned long i = opImage->getWidth() - 1; i != -1 ; i--) {
                    for(unsigned long j = opImage->getHeight() - 1; j != -1 ; j--)
                        if(ppFunc(opImage->getPixel(i, j))) {
                            SI3DPOINT rlTmpPoint = {(long) i, (long) j};
                            if(alBorder->back().x != rlTmpPoint.x || alBorder->back().y != rlTmpPoint.y) // If this point is different from the previous one, in other words if their coordinates X and/or Y are different, then It adds the point to the border.
                                alBorder->push_back(rlTmpPoint);
                            blInd = true;
                            break;
                        }
                    if(blInd) {
                        // On the selected line searches from bottom to top until It finds the first point that satisfies ppFunc taht will serve as the second point of this segment.
                        for(unsigned long j = 0; j < opImage->getHeight(); j++)
                            if(ppFunc(opImage->getPixel(i, j))) {
                                SI3DPOINT rlTmpPoint = {(long) i, (long) j};
                                if(alBorder->back().y != rlTmpPoint.y) // If this point is different from the previous one, in other words if their coordinates X and/or Y are different, then It adds the point to the border.
                                    alBorder->push_back(rlTmpPoint);
                                break;
                            }
                        break;
                    }
                }

                // Searches on the image on the line at 45 degrees with respect to the X axis and at 90 degrees with respect to the line that intersects the bottom right corner, going on from right to left and from top to bottom until It finds the first point that satisfies ppFunc that will serve as the first point of this segment.
                blInd = false;
                for(unsigned long io = opImage->getWidth() - 1, jo = 0; io != -1; io--) {
                    bool blLoop = true;
                    while(jo < opImage->getHeight() && blLoop) {
                        for(unsigned long i = io, j = jo; i != -1 && j != -1; i--, j--)
                            if(ppFunc(opImage->getPixel(i, j))) {
                                SI3DPOINT rlTmpPoint = {(long) i, (long) j};
                                if(alBorder->back().x != rlTmpPoint.x || alBorder->back().y != rlTmpPoint.y) // If this point is different from the previous one, in other words if their coordinates X and/or Y are different, then It adds the point to the border.
                                    alBorder->push_back(rlTmpPoint);
                                blInd = true;
                                break;
                            }
                        if(blInd) {
                            // On the selected line searches from bottom to top until It finds the first point that satisfies ppFunc that will serve as the second point of this segment.
                            unsigned long ilXEnd = 0;
                            unsigned long ilYEnd = 0;
                            bool blAlt = opImage->getHeight() < opImage->getWidth();
                            if(blAlt) {
                                ilXEnd = opImage->getHeight() - jo + io;
                                if(ilXEnd >= opImage->getWidth()) {
                                    ilXEnd = opImage->getWidth() - 1;
                                    ilYEnd = opImage->getHeight() - io;
                                } else
                                    ilYEnd = opImage->getHeight() - 1;
                            } else {
                                ilYEnd = opImage->getWidth() - io + jo;
                                if(ilYEnd >= opImage->getHeight()) {
                                    ilXEnd = opImage->getHeight() - jo;
                                    ilYEnd = opImage->getWidth() - 1;
                                } else
                                    ilXEnd = opImage->getWidth() - 1;
                            }
                            for(unsigned long i = ilXEnd, j = ilYEnd; i < opImage->getWidth() && j < opImage->getHeight(); i++, j++)
                                if(ppFunc(opImage->getPixel(i, j))) {
                                    SI3DPOINT rlTmpPoint = {(long) i, (long) j};
                                    if(alBorder->back().x != rlTmpPoint.x || alBorder->back().y != rlTmpPoint.y) // If this point is different from the previous one, in other words if their coordinates X and/or Y are different, then It adds the point to the border.
                                        alBorder->push_back(rlTmpPoint);
                                    break;
                                }
                            break;
                        } else if(jo < opImage->getHeight())
                            jo++;
                        else
                            blLoop = false;
                    }
                    if(blInd)
                        break;
                }

                // Searches on the image on a line at 90 degrees with respecto to the Y axis from bottom to top and from right to left until It finds the first point that satisfies ppFunc that will serve as the first point of this segment.
                blInd = false;
                for(unsigned long j = 0; j < opImage->getHeight(); j++) {
                    for(unsigned long i = opImage->getWidth() - 1; i != -1; i--)
                        if(ppFunc(opImage->getPixel(i, j))) {
                            SI3DPOINT rlTmpPoint = {(long) i, (long) j};
                            if(alBorder->back().x != rlTmpPoint.x || alBorder->back().y != rlTmpPoint.y) // If this point is different from the previous one, in other words if their coordinates X and/or Y are different, then It adds the point to the border.
                                alBorder->push_back(rlTmpPoint);
                            blInd = true;
                            break;
                        }
                    if(blInd) {
                        // On the selected line searches from left to right until It finds the first point that satisfies ppFunc that will serve as the second point of this segment.
                        for(unsigned long i = 0; j < opImage->getHeight(); i++)
                            if(ppFunc(opImage->getPixel(i, j))) {
                                SI3DPOINT rlTmpPoint = {(long) i, (long) j};
                                if(alBorder->back().x != rlTmpPoint.x) // If this point is different from the previous one, in other words if their coordinates X and/or Y are different, then It adds the point to the border.
                                    alBorder->push_back(rlTmpPoint);
                                break;
                            }
                        break;
                    }
                }

                // Searches on the image on a line at 45 degrees with respect to the X axis and at 90 degrees with respect to the line that intersects the upper left corner, going on from right to left and from bottom to top until It finds the first point that satisfies ppFunc that will serve as the first point of this segment.
                blInd = false;
                for(unsigned long io = 0, jo = 0; jo < opImage->getHeight(); jo++) {
                    bool blLoop = true;
                    while(io < opImage->getWidth() && blLoop) {
                        for(unsigned long i = io, j = jo; i != -1 && j < opImage->getHeight(); i--, j++)
                            if(ppFunc(opImage->getPixel(i, j))) {
                                SI3DPOINT rlTmpPoint = {(long) i, (long) j};
                                if(alBorder->back().x != rlTmpPoint.x || alBorder->back().y != rlTmpPoint.y) // If this point is different from the previous one, in other words if their coordinates X and/or Y are different, then It adds the point to the border.
                                    alBorder->push_back(rlTmpPoint);
                                blInd = true;
                                break;
                            }
                        if(blInd) {
                            // On the selected line searches from top to bottom until It finds the first point that satisfies ppFunc that will serve as the second point of this segment.
                            unsigned long ilXEnd = 0;
                            unsigned long ilYEnd = 0;
                            bool blAlt = opImage->getHeight() < opImage->getWidth();
                            if(blAlt) {
                                ilXEnd = opImage->getHeight() - jo + io;
                                if(ilXEnd >= opImage->getWidth()) {
                                    ilXEnd = opImage->getWidth() - 1;
                                    ilYEnd = opImage->getHeight() - io;
                                } else
                                    ilYEnd = opImage->getHeight() - 1;
                            } else {
                                ilYEnd = opImage->getWidth() - io + jo;
                                if(ilYEnd >= opImage->getHeight()) {
                                    ilXEnd = opImage->getHeight() - jo;
                                    ilYEnd = opImage->getWidth() - 1;
                                } else
                                    ilXEnd = opImage->getWidth() - 1;
                            }
                            for(unsigned long i = ilXEnd, j = ilYEnd; i < opImage->getWidth() && j != -1; i++, j--)
                                if(ppFunc(opImage->getPixel(i, j))) {
                                    SI3DPOINT rlTmpPoint = {(long) i, (long) j};
                                    if(alBorder->back().x != rlTmpPoint.x || alBorder->back().y != rlTmpPoint.y) // If this point is different from the previous one, in other words if their coordinates X and/or Y are different, then It adds the point to the border.
                                        alBorder->push_back(rlTmpPoint);
                                    break;
                                }
                            break;
                        } else if(io < opImage->getWidth())
                            io++;
                        else
                            blLoop = false;
                    }
                    if(blInd)
                        break;
                }
            }

            return alBorder;
        }

        shared_ptr <BORDER> getImageTightContourBorderByAlpha(
            shared_ptr <const BaseImage> opImage,
            unsigned char ipValidAlpha, // Minimum alpha used to search for the border.
            unsigned char ipSegmentLength, // Distance between the elements of the border, but the distance between the first and last point on the boder may be shorter.
            unsigned int ipBorderSize, // Maximum Number of points on the border, if the number of points is enough then It is possible that the distance between the first and last point on the border is greater than ipSegmentLength.
            unsigned int ipBackTrace // Number of points that are considered for proximity search.
        ) {
            return getImageTightContourBorder(
                opImage,
                [ipValidAlpha] (const RGBA& rpPoint) {
                    return rpPoint.a >= ipValidAlpha;
                },
                ipSegmentLength,
                ipBorderSize,
                ipBackTrace
            );
        }

        shared_ptr <BORDER> getImageTightContourBorderByColor(
            shared_ptr <const BaseImage> opImage,
            const RGBA& ipBackgroundColor, // This is the foreground color that will be used to search for the border.
            unsigned char ipSegmentLength, // Distance between the elements of the border, but the distance between the first and last point on the boder may be shorter.
            unsigned int ipBorderSize, // Maximum Number of points on the border, if the number of points is enough then It is possible that the distance between the first and last point on the border is greater than ipSegmentLength.
            unsigned int ipBackTrace // Number of points that are considered for proximity search.
        ) {
            return getImageTightContourBorder(
                opImage,
                [ipBackgroundColor] (const RGBA& rpPoint) {
                    return rpPoint != ipBackgroundColor;
                },
                ipSegmentLength,
                ipBorderSize,
                ipBackTrace
            );
        }

        shared_ptr <BORDER> getImageLooseContourBorderByAlpha(
            shared_ptr <const BaseImage> opImage,
            unsigned char ipValidAlpha // Minimum alpha used to search for the border.
        ) {
            return getImageLooseContourBorder(
                opImage,
                [ipValidAlpha](const RGBA& rpPoint) {
                    return rpPoint.a >= ipValidAlpha;
                }
            );
        }

        // NOTE: Even though this method can find the border on any image, the idea is to use It to find the border using the masks of the masked sprites.
        std::shared_ptr <BORDER> getImageLooseContourBorderByColor(
            std::shared_ptr <const tools::image::BaseImage> opImage,
            const tools::image::RGBA& ipBackgroundColor // This is the foreground color that will be used to search for the border.
        ) {
            return getImageLooseContourBorder(
                opImage,
                [ipBackgroundColor](const RGBA& rpPoint) {
                    return rpPoint != ipBackgroundColor;
                }
           );
        }

        BORDER_REF getImageBoxBorder(
            shared_ptr <const BaseImage> opImage,
            function <bool(const RGBA&)> ppFunc, // Function that validates if a point is on the border.
            bool bpDivideByQuandrants
        ) {
            if(!opImage)
                throw string("GameTools::getImageBoxBorder: Image cannot be null.");
            if(!ppFunc)
                throw string("GameTools::getImageBoxBorder: Evaluation Function cannot be null.");
            if(opImage->isPlaceholder())
                throw string("GameTools::getImageBoxBorder: Image is a Placeholder.");

            long ilMinX;
            long ilMaxX;
            long ilMinY;
            long ilMaxY;

            for(unsigned long i = 0; i < opImage->getWidth(); i++)
                for(unsigned long j = 0; j < opImage->getHeight(); j++)
                    if(ppFunc(opImage->getPixel(i, j)))
                        ilMinX = (long) i;

            for(unsigned long i = opImage->getWidth() - 1; i < ULONG_MAX; i--)
                for(unsigned long j = 0; j < opImage->getHeight(); j++)
                    if(ppFunc(opImage->getPixel(i, j)))
                        ilMaxX = (long) i;

            for(unsigned long j = 0; j < opImage->getHeight(); j++)
                for(unsigned long i = 0; i < opImage->getWidth(); i++)
                    if(ppFunc(opImage->getPixel(i, j)))
                        ilMinY = (long) j;

            for(unsigned long j = opImage->getHeight() - 1; j < ULONG_MAX; j--)
                for(unsigned long i = 0; i < opImage->getWidth(); i++)
                    if(ppFunc(opImage->getPixel(i, j)))
                        ilMaxY = (long) j;

            BORDER_REF alBorder;

            if(bpDivideByQuandrants) {
                long ilCenterX = (ilMaxX + ilMinX) / 2;
                long ilCenterY = (ilMaxY + ilMinY) / 2;

                alBorder.push_back(make_shared <BORDER> ());
                alBorder.back()->push_back({ilMinX, ilMinY});
                alBorder.back()->push_back({ilMinX, ilMaxY});
                alBorder.back()->push_back({ilCenterX, ilCenterY});

                alBorder.push_back(make_shared <BORDER> ());
                alBorder.back()->push_back({ilMinX, ilMaxY});
                alBorder.back()->push_back({ilMaxX, ilMaxY});
                alBorder.back()->push_back({ilCenterX, ilCenterY});

                alBorder.push_back(make_shared <BORDER> ());
                alBorder.back()->push_back({ilMaxX, ilMaxY});
                alBorder.back()->push_back({ilMaxX, ilMinY});
                alBorder.back()->push_back({ilCenterX, ilCenterY});

                alBorder.push_back(make_shared <BORDER> ());
                alBorder.back()->push_back({ilMaxX, ilMinY});
                alBorder.back()->push_back({ilMinX, ilMinY});
                alBorder.back()->push_back({ilCenterX, ilCenterY});
            } else {
                alBorder.push_back(make_shared <BORDER>());
                alBorder.back()->push_back({ilMinX, ilMinY});
                alBorder.back()->push_back({ilMinX, ilMaxY});
                alBorder.back()->push_back({ilMaxX, ilMaxY});
                alBorder.back()->push_back({ilMaxX, ilMinY});
            }

            return alBorder;
        }

        BORDER_REF getImageBoxBorderByAlpha(
            shared_ptr <const BaseImage> olImage,
            unsigned char ipValidAlpha,
            bool bpDivideByQuandrants
        ) {
            return getImageBoxBorder(
                olImage,
                [ipValidAlpha](const RGBA& rpPoint) {
                    return rpPoint.a >= ipValidAlpha;
                },
                bpDivideByQuandrants
            );
        }

        // NOTE: Even though this method can find the border on any image, the idea is to use It to find the border using the masks of the masked sprites.
        BORDER_REF getImageBoxBorderByColor(
            shared_ptr <const BaseImage> olImage,
            const RGBA& ipBackgroundColor, // This is the foreground color that will be used to search for the border.
            bool bpDivideByQuandrants
        ) {
            return getImageBoxBorder(
                olImage,
                [ipBackgroundColor](const RGBA& rpPoint) {
                    return rpPoint != ipBackgroundColor;
                },
                bpDivideByQuandrants
            );
        }

        // This method converts the real numbers into integers dealing with them as binary numbers, and then uses the point slope equation of the straight line to count all the segments in apBorder that are intersected by the segment defined by rpAPoint and rpBPoint.
        shared_ptr <BORDER> findIntersections(
            const SI3DPOINT& rpAPoint,
            const SI3DPOINT& rpBPoint,
            const shared_ptr <CONST_BORDER> apBorder,
            bool bpReturnOnFirstCross,
            bool bpCountOneSidedNodeAsOne,
            bool bpCountTwoSidedNodeAsOne,
            unsigned char ipPrecision
        ) {
            if(!apBorder)
                throw string("gametools::utilities::findIntersections: Border cannot be null.");
            if(ipPrecision < DECIMAL_CONVERSION_PRECISION)
                throw string("gametools::utilities::findIntersections: The Precision cannot be less than 3.");

            bool blAIsVertical = false;
            bool blBIsVertical = false;

            BORDER alASegment(2);
            BORDER alBSegment(2);

            // Transforms the coordinates of segment A in rpAPoint and rpBPoint into integer formated reals.
            alASegment[0].x = rpAPoint.x << ipPrecision;
            alASegment[0].y = rpAPoint.y << ipPrecision;
            alASegment[1].x = rpBPoint.x << ipPrecision;
            alASegment[1].y = rpBPoint.y << ipPrecision;

            shared_ptr <BORDER> plIntersections = make_shared <BORDER> ();

            // Go on all the segments in apBorder.
            long long ilDiff = 0;
            bool blPassEnd = false;
            for(size_t j = 1; j < apBorder->size(); j = ilDiff && j == apBorder->size() - 1? 0: j + 1) {
                if(!blPassEnd)
                    blPassEnd = j == 0;

                // Transforms the coordinates of segment B in apBorder[j - 1] and apBorder[j] into integer formated reals.
                size_t ilPreviousPoint = blPassEnd && j == 0 ? apBorder->size() - 1 : j - 1;
                alBSegment[0].x = apBorder->at(ilPreviousPoint).x << ipPrecision;
                alBSegment[0].y = apBorder->at(ilPreviousPoint).y << ipPrecision;
                alBSegment[1].x = apBorder->at(j).x << ipPrecision;
                alBSegment[1].y = apBorder->at(j).y << ipPrecision;

                // orders the points according to their coordinates from smaller to greater on each axis for bot segments.
                size_t ilMinAXInx;
                size_t ilMaxAXInx;
                if(alASegment[0].x < alASegment[1].x) {
                    ilMinAXInx = 0;
                    ilMaxAXInx = 1;
                } else {
                    ilMinAXInx = 1;
                    ilMaxAXInx = 0;
                }
                size_t ilMinBXInx;
                size_t ilMaxBXInx;
                if(alBSegment[0].x < alBSegment[1].x) {
                    ilMinBXInx = 0;
                    ilMaxBXInx = 1;
                } else {
                    ilMinBXInx = 1;
                    ilMaxBXInx = 0;
                }
                size_t ilMinAYInx;
                size_t ilMaxAYInx;
                if(alASegment[0].y < alASegment[1].y) {
                    ilMinAYInx = 0;
                    ilMaxAYInx = 1;
                } else {
                    ilMinAYInx = 1;
                    ilMaxAYInx = 0;
                }
                size_t ilMinBYInx;
                size_t ilMaxBYInx;
                if(alBSegment[0].y < alBSegment[1].y) {
                    ilMinBYInx = 0;
                    ilMaxBYInx = 1;
                } else {
                    ilMinBYInx = 1;
                    ilMaxBYInx = 0;
                }

                // If the following conditions are met then the segment B is not inside the square whose diagonal corresponds to the currecnt segment of A, and therefore they cannot intersect each other.
                if(
                    (alBSegment[0].x < alASegment[ilMinAXInx].x && alBSegment[1].x < alASegment[ilMinAXInx].x) ||
                    (alBSegment[0].x > alASegment[ilMaxAXInx].x && alBSegment[1].x > alASegment[ilMaxAXInx].x) ||
                    (alBSegment[0].y < alASegment[ilMinAYInx].y && alBSegment[1].y < alASegment[ilMinAYInx].y) ||
                    (alBSegment[0].y > alASegment[ilMaxAYInx].y && alBSegment[1].y > alASegment[ilMaxAYInx].y)
                )
                    continue;

                // Finds the differentials between the X axis and the Y axis of the points of the segment A.
                long ilYD = alASegment[ilMaxAXInx].y - alASegment[ilMinAXInx].y;
                long ilXD = alASegment[ilMaxAXInx].x - alASegment[ilMinAXInx].x;


                long ilASlope;
                if(ilXD != 0) { // Reckons the slope of the segment A if the differential on X is not 0.
                    ilASlope = (ilYD << ipPrecision) / ilXD;
                    blAIsVertical = false;
                } else { // If the differential on X is 0 then the segment A is vertical.
                    ilASlope = 0;
                    blAIsVertical = true;
                }

                // Finds the differentials between the X axis and the Y axis of the points of the segment B.
                ilYD = alBSegment[ilMaxBXInx].y - alBSegment[ilMinBXInx].y;
                ilXD = alBSegment[ilMaxBXInx].x - alBSegment[ilMinBXInx].x;

                long ilBSlope;
                if(ilXD != 0) { // Reckons the slope of the segment B if the differential on X is not 0.
                    ilBSlope = (ilYD << ipPrecision) / ilXD;
                    blBIsVertical = false;
                    // This condition makes the following condition to fail in case that A is vertical and B is not.
                    if(blAIsVertical)
                        ilASlope = ilBSlope + 1;
                } else { // If the differential on X is 0 then the segment B is vertical.
                    // This condition makes the following condition to fail in case that B is vertical and A is not.
                    ilBSlope = blAIsVertical? 0: ilASlope + 1;
                    blBIsVertical = true;
                }

                if(ilASlope - ilBSlope == 0) // If the condition is satisfied then the segments are parallel.
                    continue;

                long ilInterceptX; // X coordinate of the intersection point.
                long ilInterceptY; // Y coordinate of the intersection point.
                if(!blAIsVertical && !blBIsVertical) { // If the segments are not vertical then It finds the intersection point (ilInterceptX, ilInterceptY) according to the point slope forms of the segments, and try to find if It is on the segments.
                    unsigned long ilTruncateMask = -1 << ipPrecision;
                    unsigned long ilRoundingMask = 1 << (ipPrecision - 1);
                    unsigned long ilRoundUnit = 1 << ipPrecision;

                    ilInterceptX = ((alBSegment[ilMinBXInx].y - alASegment[ilMinAXInx].y + ((ilASlope * (long long) alASegment[ilMinAXInx].x) >> ipPrecision) - ((ilBSlope * (long long) alBSegment[ilMinBXInx].x) >> ipPrecision)) << ipPrecision) / (ilASlope - ilBSlope);

                    if(ilInterceptX < alASegment[ilMinAXInx].x || ilInterceptX > alASegment[ilMaxAXInx].x || ilInterceptX < alBSegment[ilMinBXInx].x || ilInterceptX > alBSegment[ilMaxBXInx].x) {
                        bool blRound = ilInterceptX & ilRoundingMask;
                        ilInterceptX &= ilTruncateMask; // Truncates ilInterceptX.
                        if(ilInterceptX < alASegment[ilMinAXInx].x || ilInterceptX > alASegment[ilMaxAXInx].x || ilInterceptX < alBSegment[ilMinBXInx].x || ilInterceptX > alBSegment[ilMaxBXInx].x) {
                            if(!blRound) // If rounding is not going to change ilInterceptX then It is not necessary to check It again and cotinues.
                                continue;
                            ilInterceptX += ilRoundUnit; // Round ilInterceptX.
                            if(ilInterceptX < alASegment[ilMinAXInx].x || ilInterceptX > alASegment[ilMaxAXInx].x || ilInterceptX < alBSegment[ilMinBXInx].x || ilInterceptX > alBSegment[ilMaxBXInx].x)
                                continue;
                        }
                        
                    }

                    ilInterceptY = (((ilASlope * (long long) (ilInterceptX - alASegment[ilMinAXInx].x)) >> ipPrecision) + alASegment[ilMinAXInx].y);

                    if(ilInterceptY < alASegment[ilMinAYInx].y || ilInterceptY > alASegment[ilMaxAYInx].y || ilInterceptY < alBSegment[ilMinBYInx].y || ilInterceptY > alBSegment[ilMaxBYInx].y) {
                        bool blRound = ilInterceptY & ilRoundingMask;
                        ilInterceptY &= ilTruncateMask; // Truncates ilInterceptY.
                        if(ilInterceptY < alASegment[ilMinAYInx].y || ilInterceptY > alASegment[ilMaxAYInx].y || ilInterceptY < alBSegment[ilMinBYInx].y || ilInterceptY > alBSegment[ilMaxBYInx].y) {
                            if(!blRound) // If rounding is not going to change ilInterceptY then It is not necessary to check It again and cotinues.
                                continue;
                            ilInterceptY += ilRoundUnit; // Rounds ilInterceptY.
                            if(ilInterceptY < alASegment[ilMinAYInx].y || ilInterceptY > alASegment[ilMaxAYInx].y || ilInterceptY < alBSegment[ilMinBYInx].y || ilInterceptY > alBSegment[ilMaxBYInx].y)
                                continue;
                        }
                    }
                } else if(blAIsVertical) { // If the segment A is vertical reckons the intersection point (ilInterceptX, ilInterceptY) according to the point slope equation of the segment B, and checks if the point is on B and A.
                    ilInterceptX = alASegment[0].x;

                    if(ilInterceptX < alBSegment[ilMinBXInx].x || ilInterceptX > alBSegment[ilMaxBXInx].x)
                        continue;

                    ilInterceptY = ((ilBSlope * (ilInterceptX - alBSegment[ilMinBXInx].x)) >> ipPrecision) + alBSegment[ilMinBXInx].y;

                    if(ilInterceptY < alBSegment[ilMinBYInx].y || ilInterceptY > alBSegment[ilMaxBYInx].y || ilInterceptY < alASegment[ilMinAYInx].y || ilInterceptY > alASegment[ilMaxAYInx].y)
                        continue;
                } else { // If the segment B is vertical reckons the intersection point (ilInterceptX, ilInterceptY) according to the point slope equation of the segment A, and checks if the point is on A and B.
                    ilInterceptX = alBSegment[0].x;

                    if(ilInterceptX < alASegment[ilMinAXInx].x || ilInterceptX > alASegment[ilMaxAXInx].x)
                        continue;

                    ilInterceptY = ((ilASlope * (ilInterceptX - alASegment[ilMinAXInx].x)) >> ipPrecision) + alASegment[ilMinAXInx].y;

                    if(ilInterceptY < alASegment[ilMinAYInx].y || ilInterceptY > alASegment[ilMaxAYInx].y || ilInterceptY < alBSegment[ilMinBYInx].y || ilInterceptY > alBSegment[ilMaxBYInx].y)
                        continue;
                }

                // Defines how should the segment B be treated when the segment A goes through by any of Its extremes, in other words if It should count both segments on the border that share that point as only one intersection or a two intersections.
                // In this case there are 2 types of common points:
                // 1- The points of one side (One Sided Points) are those whose segments are at the same side of the segment A.
                // 2- The points of two sides (Two Sided Points) are those whose segments are at both sides of the segment A.
                if(((ilInterceptX == alBSegment[1].x && ilInterceptY == alBSegment[1].y) || (ilInterceptX == alBSegment[0].x && ilInterceptY == alBSegment[0].y)) && (bpCountOneSidedNodeAsOne || bpCountTwoSidedNodeAsOne)) {
                    if(!blAIsVertical) { // If the segment A is not vertical then...
                        long ilPointY;

                        if(ilDiff) { // Finds out the side of A where the second segment on the border is.
                            ilPointY = ((ilASlope * (alBSegment[1].x - alASegment[ilMinAXInx].x)) >> ipPrecision) + alASegment[ilMinAXInx].y;

                            long ilLastYDiff = ilPointY - alBSegment[1].y;
                            // Constraints ilLastYDiff to be -1, 0 or 1.
                            if(ilLastYDiff != 0)
                                ilLastYDiff = ilLastYDiff > 0 ? 1 : -1;

                            if(ilDiff != ilLastYDiff) { // If they are different then the segments are at both sides of A. 
                                ilDiff = 0;
                                if(bpCountTwoSidedNodeAsOne) {
                                    if(blPassEnd) {
                                        plIntersections->pop_back();
                                        break;
                                    }
                                    continue;
                                }
                            } else { // Else the segments are at the same side of A.
                                ilDiff = 0;
                                if(bpCountOneSidedNodeAsOne) {
                                    if(blPassEnd) {
                                        plIntersections->pop_back();
                                        break;
                                    }
                                    continue;
                                }
                            }
                        } else { // First It finds out the side of A where the first segment of the border is.
                            ilPointY = ((ilASlope * (alBSegment[0].x - alASegment[ilMinAXInx].x)) >> ipPrecision) + alASegment[ilMinAXInx].y;

                            ilDiff = ilPointY - alBSegment[0].y;
                            // Constraints ilDiff to be -1, 0 or 1.
                            if(ilDiff != 0)
                                ilDiff = ilDiff > 0 ? 1 : -1;
                        }
                    } else { // Else A is vertical...
                        if(ilDiff) { // Finds out the side of A where the second segment on the border is.
                            long ilLastXDiff = alBSegment[1].x - ilInterceptX;
                            // Constraints ilLastXDiff to be -1, 0 or 1.
                            if(ilLastXDiff != 0)
                                ilLastXDiff = ilLastXDiff > 0 ? 1 : -1;

                            if(ilDiff != ilLastXDiff) { // If they are different then the segments are at both sides of A. 
                                ilDiff = 0;
                                if(bpCountTwoSidedNodeAsOne) {
                                    if(blPassEnd) {
                                        plIntersections->pop_back();
                                        break;
                                    }
                                    continue;
                                }
                            } else { // Else the segments are at the same side of A.
                                ilDiff = 0;
                                if(bpCountOneSidedNodeAsOne) {
                                    if(blPassEnd) {
                                        plIntersections->pop_back();
                                        break;
                                    }
                                    continue;
                                }
                            }
                        } else { // First It finds out the side of A where the first segment of the border is.
                            ilDiff = alBSegment[0].x - ilInterceptX;
                            // Se normaliza ilDiff para que sea -1, 0 o 1.
                            if(ilDiff != 0)
                                ilDiff = ilDiff > 0 ? 1 : -1;
                        }
                    }
                } else
                    ilDiff = 0;

                plIntersections->push_back({ilInterceptX >> ipPrecision, ilInterceptY >> ipPrecision});
                if(bpReturnOnFirstCross)
                    break;
            }

            return plIntersections;
        }

        bool isIntersectedBy(
            const shared_ptr <CONST_BORDER> apABorder,
            const shared_ptr <CONST_BORDER> apBBorder,
            unsigned char ipPrecision
        ) {
            if(!apABorder)
                throw string("ImageTools::isIntersectedBy: Border A cannot be null.");
            if(!apBBorder)
                throw string("ImageTools::isIntersectedBy: Border B cannot be null.");
            if(ipPrecision < DECIMAL_CONVERSION_PRECISION)
                throw string("ImageTools::isIntersectedBy: The Precision cannot be less than 3.");

            for(size_t i = 1; i < apABorder->size(); i++)
                if(findIntersections(apABorder->at(i - 1), apABorder->at(i), apBBorder, true, false, true, ipPrecision)->size())
                    return true;

            return false;
        }

        // This method uses the algorithm of projection to find out if the current point en apPoints is within the border defined by apBorder.
        bool isInsideOf(
            const shared_ptr <CONST_BORDER> apPoints,
            const shared_ptr <CONST_BORDER> apBorder,
            unsigned char ipPrecision
        ) {
            if(!apPoints)
                throw string("ImageTools::isInsideOf: The Set of Points cannot be null.");
            if(!apBorder)
                throw string("ImageTools::isInsideOf: The Border cannot be null.");
            if(ipPrecision < DECIMAL_CONVERSION_PRECISION)
                throw string("ImageTools::detectIntersection: The Precision cannot be less than 3.");

            SI3DPOINT rlTmp;
            rlTmp.x = 0;
            for(size_t i = 0; i < apPoints->size(); i++) {
                rlTmp.y = apPoints->at(i).y;
                if(findIntersections(rlTmp, apPoints->at(i), apBorder, false, false, true, ipPrecision)->size() & 0x00000001)
                    return true;
            }

            return false;
        }

        bool isWithinRadiusOf(
            const SI3DPOINT& rpACenter,
            unsigned short ipARadius,
            const SI3DPOINT& rpAReference,
            const SI3DPOINT& rpBCenter,
            unsigned short ipBRadius,
            const SI3DPOINT& rpBReference
        ) {
            unsigned short ilRadius = ipARadius > ipBRadius ? ipARadius : ipBRadius;
            if(sqrt(pow((rpACenter.x + rpAReference.x) - (rpBCenter.x + rpBReference.x), 2) + pow((rpACenter.y + rpAReference.y) - (rpBCenter.y + rpBReference.y), 2)) < (double) ilRadius)
                return true;
            return false;
        }

        shared_ptr <RGBAImage> getMaskedBitmap(shared_ptr <const RGBAImage> opImage, bool bpIsMask, bool blInvert, const RGBA& ipBackgroundColor) {
            if(!opImage)
                throw string("ImageTools::getMaskedBitmap: The Image cannot be null.");
            if(opImage->isPlaceholder())
                throw string("ImageTools::getMaskedBitmap: The Image is a Placeholder.");

            shared_ptr <RGBAImage> plMask = make_shared <RGBAImage> (opImage->getWidth(), opImage->getHeight());
            RGBA rlTmp;
            for(unsigned long y = 0; y < opImage->getHeight(); y++)
                for(unsigned long x = 0; x < opImage->getWidth(); x++)
                    if(opImage->getRealPixel(x, y) != ipBackgroundColor) {
                        if(bpIsMask)
                            rlTmp.r = rlTmp.g = rlTmp.b = blInvert ? 255: 0;
                        else
                            rlTmp = opImage->getRealPixel(x, y);
                        plMask->setPixel(x, y, rlTmp);
                    } else {
                        if(bpIsMask)
                            rlTmp.r = rlTmp.g = rlTmp.b = blInvert ? 0: 255;
                        else
                            rlTmp.r = rlTmp.g = rlTmp.b = blInvert ? 255: 0;
                        plMask->setPixel(x, y, rlTmp);
                    }

            return plMask;
        }

        shared_ptr <vector <float>> getPerspectiveMatrix(float fpFieldOfView, float fpAspect, float fpZNear, float fpZFar) {
            float xymax = fpZNear * (float) tan(fpFieldOfView * (M_PI / 360.0f));
            float depth = fpZFar - fpZNear;

            shared_ptr <vector <float>> alResult = make_shared <vector <float>> ();

            *alResult = {
                0.0f, 0.0f, 0.0f, 0.0f,
                0.0f, 0.0f, 0.0f, 0.0f,
                0.0f, 0.0f, 0.0f, 0.0f,
                0.0f, 0.0f, 0.0f, 0.0f
            };

            alResult->at(0) = fpZNear / (xymax * fpAspect);
            alResult->at(5) = fpZNear / xymax;
            alResult->at(10) = -(fpZFar + fpZNear) / depth;
            alResult->at(11) = -1.0f;
            alResult->at(14) = -(2.0f * fpZFar * fpZNear) / depth;
            alResult->at(15) = 1.0f;

            return alResult;
        }

        shared_ptr <vector <float>> getTransformationMatrix(float fpNewX, float fpNewY, float fpNewZ, float fpAngle, float fpXScale, float fpYScale) {
            shared_ptr <vector <float>> alResult = make_shared <vector <float>>();

            *alResult = {
                0.0f, 0.0f, 0.0f, 0.0f,
                0.0f, 0.0f, 0.0f, 0.0f,
                0.0f, 0.0f, 0.0f, 0.0f,
                0.0f, 0.0f, 0.0f, 0.0f
            };

            alResult->at(0) = cos(fpAngle) * fpXScale;
            alResult->at(1) = sin(fpAngle) * fpXScale;
            alResult->at(4) = -sin(fpAngle) * fpYScale;
            alResult->at(5) = cos(fpAngle) * fpYScale;
            alResult->at(10) = 1;
            alResult->at(12) = fpNewX;
            alResult->at(13) = fpNewY;
            alResult->at(14) = fpNewZ;
            alResult->at(15) = 1;

            return alResult;
        }

        shared_ptr <vector <float>> getIdentityMatrix() {
            shared_ptr <vector <float>> alResult = make_shared <vector <float>>();

            *alResult = {
                1.0f, 0.0f, 0.0f, 0.0f,
                0.0f, 1.0f, 0.0f, 0.0f,
                0.0f, 0.0f, 1.0f, 0.0f,
                0.0f, 0.0f, 0.0f, 1.0f
            };

            return alResult;
        }

        SI3DPOINT getNewVector(long fpX, long fpY, long fpZ, long ipNewX, long ipNewY, long ipNewZ, float fpAngle, float fpXScale, float fpYScale) {
            return {static_cast <long>(ipNewX + (fpYScale * fpY * sin(fpAngle)) + (fpXScale * fpX * cos(fpAngle))), static_cast <long>(ipNewY - (fpXScale * fpX * sin(fpAngle)) + (fpYScale * fpY * cos(fpAngle))), ipNewZ};
        }

        wstring convertoToWString(const string& spSource) {
            size_t ilConvCharCnt;
            size_t ilMaxSize = spSource.size() * 2;
            vector <wchar_t> slBuff(ilMaxSize);
            mbstowcs_s(&ilConvCharCnt, &slBuff[0], (sizeof(wchar_t) * ilMaxSize) / 2, spSource.c_str(), ilMaxSize);
            wstring wslDestiny(&slBuff[0]);

            return wslDestiny;
        }

        string convertoToString(const wstring& spSource) {
            size_t ilConvCharCnt;
            size_t ipMaxSize = spSource.size() * 4;
            vector <char> slBuff(ipMaxSize);
            wcstombs_s(&ilConvCharCnt, &slBuff[0], ipMaxSize, spSource.c_str(), ipMaxSize);
            string slDestiny(&slBuff[0]);

            return slDestiny;
        }

    } // END UTILITIES

} // END GAMELIB

//////////////////////////////////////////////////////////////////////////////////////