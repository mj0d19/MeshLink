<?php

use Illuminate\Support\Facades\Route;
use App\Http\Controllers\Api\RollCallController;
use App\Http\Controllers\DashboardController;
use App\Http\Controllers\AttendanceSnapshotController;

Route::get('/', [DashboardController::class, 'index'])->name('dashboard');

// Route::get('/', function () {
//     return view('welcome');
// });
Route::post('/attendance-snapshots', [AttendanceSnapshotController::class, 'store'])
    ->name('attendance-snapshots.store');
Route::get('/attendance-snapshots/{snapshot}', [AttendanceSnapshotController::class, 'show'])
    ->name('attendance-snapshots.show');
Route::get('/store', [RollCallController::class, 'store']);
Route::get('/status', [RollCallController::class, 'status']);
Route::get('/checkpoint', [RollCallController::class, 'saveCheckpoint']);