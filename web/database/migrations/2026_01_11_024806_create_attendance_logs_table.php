<?php

use Illuminate\Database\Migrations\Migration;
use Illuminate\Database\Schema\Blueprint;
use Illuminate\Support\Facades\Schema;

return new class extends Migration
{
    /**
     * Run the migrations.
     */
    public function up(): void
    {
        Schema::create('attendance_logs', function (Blueprint $table) {
            $table->id();
            $table->foreignId('checkpoint_id')->nullable()->constrained()->nullOnDelete();
            $table->foreignId('device_id')->constrained()->cascadeOnDelete();

            $table->timestamp('seen_at');             // وقت ما انشاف
            $table->integer('rssi')->nullable();      // قوة الإشارة (اختياري)
            $table->string('source_uid')->nullable(); // UID حق الليدر/الماسح (اختياري)

            $table->timestamps();

            $table->index(['checkpoint_id', 'device_id']);
            $table->index(['seen_at']);
        });
    }

    /**
     * Reverse the migrations.
     */
    public function down(): void
    {
        Schema::dropIfExists('attendance_logs');
    }
};
