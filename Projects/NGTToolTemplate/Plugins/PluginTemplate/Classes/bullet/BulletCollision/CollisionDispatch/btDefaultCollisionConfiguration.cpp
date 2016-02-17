/*==================================================================================
    Copyright (c) 2008, binaryzebra
    All rights reserved.

    Redistribution and use in source and binary forms, with or without
    modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright
    notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright
    notice, this list of conditions and the following disclaimer in the
    documentation and/or other materials provided with the distribution.
    * Neither the name of the binaryzebra nor the
    names of its contributors may be used to endorse or promote products
    derived from this software without specific prior written permission.

    THIS SOFTWARE IS PROVIDED BY THE binaryzebra AND CONTRIBUTORS "AS IS" AND
    ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
    WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
    DISCLAIMED. IN NO EVENT SHALL binaryzebra BE LIABLE FOR ANY
    DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
    (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
    LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
    ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
    (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
    SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
=====================================================================================*/

/*
Bullet Continuous Collision Detection and Physics Library
Copyright (c) 2003-2006 Erwin Coumans  http://continuousphysics.com/Bullet/

This software is provided 'as-is', without any express or implied warranty.
In no event will the authors be held liable for any damages arising from the use of this software.
Permission is granted to anyone to use this software for any purpose, 
including commercial applications, and to alter it and redistribute it freely, 
subject to the following restrictions:

1. The origin of this software must not be misrepresented; you must not claim that you wrote the original software. If you use this software in a product, an acknowledgment in the product documentation would be appreciated but is not required.
2. Altered source versions must be plainly marked as such, and must not be misrepresented as being the original software.
3. This notice may not be removed or altered from any source distribution.
*/

#include "btDefaultCollisionConfiguration.h"

#include "bullet/BulletCollision/CollisionDispatch/btConvexConvexAlgorithm.h"
#include "bullet/BulletCollision/CollisionDispatch/btEmptyCollisionAlgorithm.h"
#include "bullet/BulletCollision/CollisionDispatch/btConvexConcaveCollisionAlgorithm.h"
#include "bullet/BulletCollision/CollisionDispatch/btCompoundCollisionAlgorithm.h"
#include "bullet/BulletCollision/CollisionDispatch/btConvexPlaneCollisionAlgorithm.h"
#include "bullet/BulletCollision/CollisionDispatch/btBoxBoxCollisionAlgorithm.h"
#include "bullet/BulletCollision/CollisionDispatch/btSphereSphereCollisionAlgorithm.h"
#ifdef USE_BUGGY_SPHERE_BOX_ALGORITHM
#include "BulletCollision/CollisionDispatch/btSphereBoxCollisionAlgorithm.h"
#endif //USE_BUGGY_SPHERE_BOX_ALGORITHM
#include "bullet/BulletCollision/CollisionDispatch/btSphereTriangleCollisionAlgorithm.h"
#include "bullet/BulletCollision/NarrowPhaseCollision/btGjkEpaPenetrationDepthSolver.h"
#include "bullet/BulletCollision/NarrowPhaseCollision/btMinkowskiPenetrationDepthSolver.h"
#include "bullet/BulletCollision/NarrowPhaseCollision/btVoronoiSimplexSolver.h"



#include "bullet/LinearMath/btStackAlloc.h"
#include "bullet/LinearMath/btPoolAllocator.h"

btDefaultCollisionConfiguration::btDefaultCollisionConfiguration(const btDefaultCollisionConstructionInfo& constructionInfo)
//btDefaultCollisionConfiguration::btDefaultCollisionConfiguration(btStackAlloc*	stackAlloc,btPoolAllocator*	persistentManifoldPool,btPoolAllocator*	collisionAlgorithmPool)
{
    void* mem = btAlignedAlloc(sizeof(btVoronoiSimplexSolver), 16);
    m_simplexSolver = new (mem) btVoronoiSimplexSolver();

    if (constructionInfo.m_useEpaPenetrationAlgorithm)
    {
        mem = btAlignedAlloc(sizeof(btGjkEpaPenetrationDepthSolver), 16);
        m_pdSolver = new (mem) btGjkEpaPenetrationDepthSolver;
    }
    else
    {
        mem = btAlignedAlloc(sizeof(btMinkowskiPenetrationDepthSolver), 16);
        m_pdSolver = new (mem) btMinkowskiPenetrationDepthSolver;
    }

    //default CreationFunctions, filling the m_doubleDispatch table
    mem = btAlignedAlloc(sizeof(btConvexConvexAlgorithm::CreateFunc), 16);
    m_convexConvexCreateFunc = new (mem) btConvexConvexAlgorithm::CreateFunc(m_simplexSolver, m_pdSolver);
    mem = btAlignedAlloc(sizeof(btConvexConcaveCollisionAlgorithm::CreateFunc), 16);
    m_convexConcaveCreateFunc = new (mem) btConvexConcaveCollisionAlgorithm::CreateFunc;
    mem = btAlignedAlloc(sizeof(btConvexConcaveCollisionAlgorithm::CreateFunc), 16);
    m_swappedConvexConcaveCreateFunc = new (mem) btConvexConcaveCollisionAlgorithm::SwappedCreateFunc;
    mem = btAlignedAlloc(sizeof(btCompoundCollisionAlgorithm::CreateFunc), 16);
    m_compoundCreateFunc = new (mem) btCompoundCollisionAlgorithm::CreateFunc;
    mem = btAlignedAlloc(sizeof(btCompoundCollisionAlgorithm::SwappedCreateFunc), 16);
    m_swappedCompoundCreateFunc = new (mem) btCompoundCollisionAlgorithm::SwappedCreateFunc;
    mem = btAlignedAlloc(sizeof(btEmptyAlgorithm::CreateFunc), 16);
    m_emptyCreateFunc = new (mem) btEmptyAlgorithm::CreateFunc;

    mem = btAlignedAlloc(sizeof(btSphereSphereCollisionAlgorithm::CreateFunc), 16);
    m_sphereSphereCF = new (mem) btSphereSphereCollisionAlgorithm::CreateFunc;
#ifdef USE_BUGGY_SPHERE_BOX_ALGORITHM
    mem = btAlignedAlloc(sizeof(btSphereBoxCollisionAlgorithm::CreateFunc), 16);
    m_sphereBoxCF = new (mem) btSphereBoxCollisionAlgorithm::CreateFunc;
    mem = btAlignedAlloc(sizeof(btSphereBoxCollisionAlgorithm::CreateFunc), 16);
    m_boxSphereCF = new (mem) btSphereBoxCollisionAlgorithm::CreateFunc;
    m_boxSphereCF->m_swapped = true;
#endif //USE_BUGGY_SPHERE_BOX_ALGORITHM

    mem = btAlignedAlloc(sizeof(btSphereTriangleCollisionAlgorithm::CreateFunc), 16);
    m_sphereTriangleCF = new (mem) btSphereTriangleCollisionAlgorithm::CreateFunc;
    mem = btAlignedAlloc(sizeof(btSphereTriangleCollisionAlgorithm::CreateFunc), 16);
    m_triangleSphereCF = new (mem) btSphereTriangleCollisionAlgorithm::CreateFunc;
    m_triangleSphereCF->m_swapped = true;

    mem = btAlignedAlloc(sizeof(btBoxBoxCollisionAlgorithm::CreateFunc), 16);
    m_boxBoxCF = new (mem) btBoxBoxCollisionAlgorithm::CreateFunc;

    //convex versus plane
    mem = btAlignedAlloc(sizeof(btConvexPlaneCollisionAlgorithm::CreateFunc), 16);
    m_convexPlaneCF = new (mem) btConvexPlaneCollisionAlgorithm::CreateFunc;
    mem = btAlignedAlloc(sizeof(btConvexPlaneCollisionAlgorithm::CreateFunc), 16);
    m_planeConvexCF = new (mem) btConvexPlaneCollisionAlgorithm::CreateFunc;
    m_planeConvexCF->m_swapped = true;

    ///calculate maximum element size, big enough to fit any collision algorithm in the memory pool
    int maxSize = sizeof(btConvexConvexAlgorithm);
    int maxSize2 = sizeof(btConvexConcaveCollisionAlgorithm);
    int maxSize3 = sizeof(btCompoundCollisionAlgorithm);
    int sl = sizeof(btConvexSeparatingDistanceUtil);
    sl = sizeof(btGjkPairDetector);
    int collisionAlgorithmMaxElementSize = btMax(maxSize, constructionInfo.m_customCollisionAlgorithmMaxElementSize);
    collisionAlgorithmMaxElementSize = btMax(collisionAlgorithmMaxElementSize, maxSize2);
    collisionAlgorithmMaxElementSize = btMax(collisionAlgorithmMaxElementSize, maxSize3);

    if (constructionInfo.m_stackAlloc)
    {
        m_ownsStackAllocator = false;
        this->m_stackAlloc = constructionInfo.m_stackAlloc;
    }
    else
    {
        m_ownsStackAllocator = true;
        void* mem = btAlignedAlloc(sizeof(btStackAlloc), 16);
        m_stackAlloc = new (mem) btStackAlloc(constructionInfo.m_defaultStackAllocatorSize);
    }

    if (constructionInfo.m_persistentManifoldPool)
    {
        m_ownsPersistentManifoldPool = false;
        m_persistentManifoldPool = constructionInfo.m_persistentManifoldPool;
    }
    else
    {
        m_ownsPersistentManifoldPool = true;
        void* mem = btAlignedAlloc(sizeof(btPoolAllocator), 16);
        m_persistentManifoldPool = new (mem) btPoolAllocator(sizeof(btPersistentManifold), constructionInfo.m_defaultMaxPersistentManifoldPoolSize);
    }

    if (constructionInfo.m_collisionAlgorithmPool)
    {
        m_ownsCollisionAlgorithmPool = false;
        m_collisionAlgorithmPool = constructionInfo.m_collisionAlgorithmPool;
    }
    else
    {
        m_ownsCollisionAlgorithmPool = true;
        void* mem = btAlignedAlloc(sizeof(btPoolAllocator), 16);
        m_collisionAlgorithmPool = new (mem) btPoolAllocator(collisionAlgorithmMaxElementSize, constructionInfo.m_defaultMaxCollisionAlgorithmPoolSize);
    }
}

btDefaultCollisionConfiguration::~btDefaultCollisionConfiguration()
{
    if (m_ownsStackAllocator)
    {
        m_stackAlloc->destroy();
        m_stackAlloc->~btStackAlloc();
        btAlignedFree(m_stackAlloc);
    }
    if (m_ownsCollisionAlgorithmPool)
    {
        m_collisionAlgorithmPool->~btPoolAllocator();
        btAlignedFree(m_collisionAlgorithmPool);
    }
    if (m_ownsPersistentManifoldPool)
    {
        m_persistentManifoldPool->~btPoolAllocator();
        btAlignedFree(m_persistentManifoldPool);
    }

    m_convexConvexCreateFunc->~btCollisionAlgorithmCreateFunc();
    btAlignedFree(m_convexConvexCreateFunc);

    m_convexConcaveCreateFunc->~btCollisionAlgorithmCreateFunc();
    btAlignedFree(m_convexConcaveCreateFunc);
    m_swappedConvexConcaveCreateFunc->~btCollisionAlgorithmCreateFunc();
    btAlignedFree(m_swappedConvexConcaveCreateFunc);

    m_compoundCreateFunc->~btCollisionAlgorithmCreateFunc();
    btAlignedFree(m_compoundCreateFunc);

    m_swappedCompoundCreateFunc->~btCollisionAlgorithmCreateFunc();
    btAlignedFree(m_swappedCompoundCreateFunc);

    m_emptyCreateFunc->~btCollisionAlgorithmCreateFunc();
    btAlignedFree(m_emptyCreateFunc);

    m_sphereSphereCF->~btCollisionAlgorithmCreateFunc();
    btAlignedFree(m_sphereSphereCF);

#ifdef USE_BUGGY_SPHERE_BOX_ALGORITHM
    m_sphereBoxCF->~btCollisionAlgorithmCreateFunc();
    btAlignedFree(m_sphereBoxCF);
    m_boxSphereCF->~btCollisionAlgorithmCreateFunc();
    btAlignedFree(m_boxSphereCF);
#endif //USE_BUGGY_SPHERE_BOX_ALGORITHM

    m_sphereTriangleCF->~btCollisionAlgorithmCreateFunc();
    btAlignedFree(m_sphereTriangleCF);
    m_triangleSphereCF->~btCollisionAlgorithmCreateFunc();
    btAlignedFree(m_triangleSphereCF);
    m_boxBoxCF->~btCollisionAlgorithmCreateFunc();
    btAlignedFree(m_boxBoxCF);

    m_convexPlaneCF->~btCollisionAlgorithmCreateFunc();
    btAlignedFree(m_convexPlaneCF);
    m_planeConvexCF->~btCollisionAlgorithmCreateFunc();
    btAlignedFree(m_planeConvexCF);

    m_simplexSolver->~btVoronoiSimplexSolver();
    btAlignedFree(m_simplexSolver);

    m_pdSolver->~btConvexPenetrationDepthSolver();

    btAlignedFree(m_pdSolver);
}

btCollisionAlgorithmCreateFunc* btDefaultCollisionConfiguration::getCollisionAlgorithmCreateFunc(int proxyType0, int proxyType1)
{
    if ((proxyType0 == SPHERE_SHAPE_PROXYTYPE) && (proxyType1 == SPHERE_SHAPE_PROXYTYPE))
    {
        return m_sphereSphereCF;
    }
#ifdef USE_BUGGY_SPHERE_BOX_ALGORITHM
    if ((proxyType0 == SPHERE_SHAPE_PROXYTYPE) && (proxyType1 == BOX_SHAPE_PROXYTYPE))
    {
        return m_sphereBoxCF;
    }

    if ((proxyType0 == BOX_SHAPE_PROXYTYPE) && (proxyType1 == SPHERE_SHAPE_PROXYTYPE))
    {
        return m_boxSphereCF;
    }
#endif //USE_BUGGY_SPHERE_BOX_ALGORITHM

    if ((proxyType0 == SPHERE_SHAPE_PROXYTYPE) && (proxyType1 == TRIANGLE_SHAPE_PROXYTYPE))
    {
        return m_sphereTriangleCF;
    }

    if ((proxyType0 == TRIANGLE_SHAPE_PROXYTYPE) && (proxyType1 == SPHERE_SHAPE_PROXYTYPE))
    {
        return m_triangleSphereCF;
    }

    if ((proxyType0 == BOX_SHAPE_PROXYTYPE) && (proxyType1 == BOX_SHAPE_PROXYTYPE))
    {
        return m_boxBoxCF;
    }

    if (btBroadphaseProxy::isConvex(proxyType0) && (proxyType1 == STATIC_PLANE_PROXYTYPE))
    {
        return m_convexPlaneCF;
    }

    if (btBroadphaseProxy::isConvex(proxyType1) && (proxyType0 == STATIC_PLANE_PROXYTYPE))
    {
        return m_planeConvexCF;
    }

    if (btBroadphaseProxy::isConvex(proxyType0) && btBroadphaseProxy::isConvex(proxyType1))
    {
        return m_convexConvexCreateFunc;
    }

    if (btBroadphaseProxy::isConvex(proxyType0) && btBroadphaseProxy::isConcave(proxyType1))
    {
        return m_convexConcaveCreateFunc;
    }

    if (btBroadphaseProxy::isConvex(proxyType1) && btBroadphaseProxy::isConcave(proxyType0))
    {
        return m_swappedConvexConcaveCreateFunc;
    }

    if (btBroadphaseProxy::isCompound(proxyType0))
    {
        return m_compoundCreateFunc;
    }
    else
    {
        if (btBroadphaseProxy::isCompound(proxyType1))
        {
            return m_swappedCompoundCreateFunc;
        }
    }

    //failed to find an algorithm
    return m_emptyCreateFunc;
}

void btDefaultCollisionConfiguration::setConvexConvexMultipointIterations(int numPerturbationIterations, int minimumPointsPerturbationThreshold)
{
    btConvexConvexAlgorithm::CreateFunc* convexConvex = (btConvexConvexAlgorithm::CreateFunc*)m_convexConvexCreateFunc;
    convexConvex->m_numPerturbationIterations = numPerturbationIterations;
    convexConvex->m_minimumPointsPerturbationThreshold = minimumPointsPerturbationThreshold;
}

void btDefaultCollisionConfiguration::setPlaneConvexMultipointIterations(int numPerturbationIterations, int minimumPointsPerturbationThreshold)
{
    btConvexPlaneCollisionAlgorithm::CreateFunc* cpCF = (btConvexPlaneCollisionAlgorithm::CreateFunc*)m_convexPlaneCF;
    cpCF->m_numPerturbationIterations = numPerturbationIterations;
    cpCF->m_minimumPointsPerturbationThreshold = minimumPointsPerturbationThreshold;

    btConvexPlaneCollisionAlgorithm::CreateFunc* pcCF = (btConvexPlaneCollisionAlgorithm::CreateFunc*)m_planeConvexCF;
    pcCF->m_numPerturbationIterations = numPerturbationIterations;
    pcCF->m_minimumPointsPerturbationThreshold = minimumPointsPerturbationThreshold;
}
