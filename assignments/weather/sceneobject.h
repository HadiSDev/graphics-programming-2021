//
// Created by hadis on 09/10/2021.
//

#ifndef SCENEOBJECT_H
#define SCENEOBJECT_H
struct SceneObject{
    unsigned int VAO;
    unsigned int vertexCount;
    void drawSceneObject() const{
        glBindVertexArray(VAO);
        glDrawElements(GL_TRIANGLES,  vertexCount, GL_UNSIGNED_INT, 0);
    }
};

#endif //SCENEOBJECT_H
