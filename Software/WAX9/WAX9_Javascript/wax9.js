const ahrs = require("ahrs");

const AccNorm = 1.0 / 4096.0
const GyroNorm = 0.07
const MagNorm = 0.1

class WAX9 {
    constructor(sampleRate) {
        this.lastSampleNumber = 0;
        this.sampleRate = sampleRate;
        this.madgwick = new ahrs({
            sampleInterval: 1000 / sampleRate,
            algorithm: "Madgwick",
            beta: 0.4
        });
    }

    get quaternion() {
        let q = this.madgwick.getQuaternion();
        return {
            x: q.w,
            y: q.x,
            z: q.y,
            w: q.z
        };
    }

    update(sn, ax, ay, az, gx, gy, gz, mx, my, mz) {
        this.lastSample = sn;
        this.acc = { x: ax, y: ay, z: az };
        this.gyro = { x: gx, y: gy, z: gz };
        this.mag = { x: mx, y: my, z: mz };

        let diff;
        if (sn < this.lastSampleNumber) {
            diff = 65536 - this.lastSampleNumber + sn;
        } else {
            diff = sn - this.lastSampleNumber;
        }

        if (diff > 1) {
            console.log(`DROPPED PACKETS: ${diff}`);
        }

        this.lastSampleNumber = sn;

        if (diff > this.sampleRate) {
            diff = this.sampleRate;
        }

        let rgx = WAX9._toRadians(gx);
        let rgy = WAX9._toRadians(gy);
        let rgz = WAX9._toRadians(gz);
        
        for (let i = 0; i < diff; i++) {
            this.madgwick.update(rgx, rgy, rgz, ax, ay, az);
        }
        
        this.euler = WAX9._quatToEuler(this.quaternion);
        this.grav = WAX9._quatToGravity(this.quaternion);

        let matrixInvRotation = WAX9._matrixLoadZXYInverseRotation(this.euler.z, this.euler.y, this.euler.x);
        this.grav = WAX9._point3MultiplyMatrix([0.0, -1.0, 0.0], matrixInvRotation);

        let relAccel = [];
        relAccel[0] = this.acc.y + this.grav.x;
        relAccel[1] = this.acc.z + this.grav.y;
        relAccel[2] = this.acc.x + this.grav.z;

        let matrixRotation = WAX9._matrixLoadZXYRotation(-this.euler.z, -this.euler.y, -this.euler.x);

        this.linAcc = WAX9._point3MultiplyMatrix(relAccel, matrixRotation);
    }

    updateFromBytes(data) {
        let sn = data.readUInt16LE(0);

        let ax = data.readInt16LE(2) * AccNorm;
	    let ay = data.readInt16LE(4) * AccNorm;
	    let az = data.readInt16LE(6) * AccNorm;

	    let gx = data.readInt16LE( 8) * GyroNorm;
	    let gy = data.readInt16LE(10) * GyroNorm;
	    let gz = data.readInt16LE(12) * GyroNorm;

	    let mx = data.readInt16LE(14) * MagNorm;
	    let my = data.readInt16LE(16) * MagNorm;
	    let mz = data.readInt16LE(18) * MagNorm;

        this.update(sn, ax, ay, az, gx, gy, gz, mx, my, mz);
    }

    static _quatToEuler(q)
    {
        let x = Math.atan2(2 * q.y * q.z - 2 * q.x * q.w, 2 * q.x * q.x + 2 * q.y * q.y - 1); // psi
        let y = -Math.asin(2 * q.y * q.w + 2 * q.x * q.z); // theta
        let z = Math.atan2(2 * q.z * q.w - 2 * q.x * q.y, 2 * q.x * q.x + 2 * q.w * q.w - 1); // phi
        return { x: x, y: y, z: z };
    }

    static _quatToGravity(q) {
        let gx = 2 * (q.y * q.w - q.x * q.z);
        let gy = 2 * (q.x * q.y + q.z * q.w);
        let gz = q.x * q.x - q.y * q.y - q.z * q.z + q.w * q.w;

        return { x: -gy, y: -gz, z: -gx };
    }

    static _toRadians(x) {
        return x * Math.PI / 180.0;
    }

    static _matrixLoadZXYRotation(phi, theta, psi)
    {
        let rotZ = WAX9._matrixLoadZRotation(phi);    // Z: phi (roll)
        let rotX = WAX9._matrixLoadXRotation(theta);  // X: theta (pitch)
        let rotY = WAX9._matrixLoadYRotation(psi);    // Y: psi (yaw)
        let temp = WAX9._matrixMultiply(rotZ, rotX);
        let matrix = WAX9._matrixMultiply(temp, rotY);
        return matrix;
    }

    static _matrixLoadZXYInverseRotation(phi, theta, psi)
    {
        let rotZ = WAX9._matrixLoadZRotation(phi);    // Z: phi (roll)
        let rotX = WAX9._matrixLoadXRotation(theta);  // X: theta (pitch)
        let rotY = WAX9._matrixLoadYRotation(psi);    // Y: psi (yaw)
        let temp = WAX9._matrixMultiply(rotY, rotX);
        let matrix = WAX9._matrixMultiply(temp, rotZ);
        return matrix;
    }

    static _matrixLoadXRotation(angleRadians)
    {
        let dest = [];
        dest[0] = 1.0; dest[4] = 0.0; dest[8] = 0.0; dest[12] = 0.0;
        dest[1] = 0.0; dest[5] = Math.cos(angleRadians); dest[9] = -Math.sin(angleRadians); dest[13] = 0.0;
        dest[2] = 0.0; dest[6] = Math.sin(angleRadians); dest[10] = Math.cos(angleRadians); dest[14] = 0.0;
        dest[3] = 0.0; dest[7] = 0.0; dest[11] = 0.0; dest[15] = 1.0;
        return dest;
    }

    static _matrixLoadYRotation(angleRadians)
    {
        let dest = [];
        dest[0] = Math.cos(angleRadians); dest[4] = 0.0; dest[8] = Math.sin(angleRadians); dest[12] = 0.0;
        dest[1] = 0.0; dest[5] = 1.0; dest[9] = 0.0; dest[13] = 0.0;
        dest[2] = -Math.sin(angleRadians); dest[6] = 0.0; dest[10] = Math.cos(angleRadians); dest[14] = 0.0;
        dest[3] = 0.0; dest[7] = 0.0; dest[11] = 0.0; dest[15] = 1.0;
        return dest;
    }

    static _matrixLoadZRotation(angleRadians)
    {
        let dest = [];
        dest[0] = Math.cos(angleRadians); dest[4] = -Math.sin(angleRadians); dest[8] = 0.0; dest[12] = 0.0;
        dest[1] = Math.sin(angleRadians); dest[5] = Math.cos(angleRadians); dest[9] = 0.0; dest[13] = 0.0;
        dest[2] = 0.0; dest[6] = 0.0; dest[10] = 1.0; dest[14] = 0.0;
        dest[3] = 0.0; dest[7] = 0.0; dest[11] = 0.0; dest[15] = 1.0;
        return dest;
    }
    
    static _matrixMultiply(matrix1, matrix2)
    {
        let result = [];
        let a1, b1, c1, d1, e1, f1, g1, h1, i1, j1, k1, l1;
        let a2, b2, c2, d2, e2, f2, g2, h2, i2, j2, k2, l2;
        
        a1 = matrix1[0]; b1 = matrix1[4]; c1 = matrix1[8]; d1 = matrix1[12];
        e1 = matrix1[1]; f1 = matrix1[5]; g1 = matrix1[9]; h1 = matrix1[13];
        i1 = matrix1[2]; j1 = matrix1[6]; k1 = matrix1[10]; l1 = matrix1[14];
        a2 = matrix2[0]; b2 = matrix2[4]; c2 = matrix2[8]; d2 = matrix2[12];
        e2 = matrix2[1]; f2 = matrix2[5]; g2 = matrix2[9]; h2 = matrix2[13];
        i2 = matrix2[2]; j2 = matrix2[6]; k2 = matrix2[10]; l2 = matrix2[14];
        result[0] = a1 * a2 + b1 * e2 + c1 * i2;
        result[1] = e1 * a2 + f1 * e2 + g1 * i2;
        result[2] = i1 * a2 + j1 * e2 + k1 * i2;
        result[3] = 0.0;
        result[4] = a1 * b2 + b1 * f2 + c1 * j2;
        result[5] = e1 * b2 + f1 * f2 + g1 * j2;
        result[6] = i1 * b2 + j1 * f2 + k1 * j2;
        result[7] = 0.0;
        result[8] = a1 * c2 + b1 * g2 + c1 * k2;
        result[9] = e1 * c2 + f1 * g2 + g1 * k2;
        result[10] = i1 * c2 + j1 * g2 + k1 * k2;
        result[11] = 0.0;
        result[12] = a1 * d2 + b1 * h2 + c1 * l2 + d1;
        result[13] = e1 * d2 + f1 * h2 + g1 * l2 + h1;
        result[14] = i1 * d2 + j1 * h2 + k1 * l2 + l1;
        result[15] = 1.0;
        return result;
    }

    static _point3MultiplyMatrix(point, matrix)
    {
        let x = matrix[0] * point[0] + matrix[4] * point[1] + matrix[8] * point[2] + matrix[12];
        let y = matrix[1] * point[0] + matrix[5] * point[1] + matrix[9] * point[2] + matrix[13];
        let z = matrix[2] * point[0] + matrix[6] * point[1] + matrix[10] * point[2] + matrix[14];
        return { x: x, y: y, z: z };
    }
}

module.exports = WAX9;